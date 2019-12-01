#!/usr/bin/env python2.7
"""
 Copyright (C) 2018-2019 Intel Corporation

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
"""
from __future__ import print_function, division

import logging
import os
import sys
from argparse import ArgumentParser, SUPPRESS
from math import exp as exp
# from time 
import time

import cv2
from openvino.inference_engine import IENetwork, IECore

import numpy as np

import rospy
from std_msgs.msg import String,UInt8
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError


from pub_rospy.msg import pubmsg
from pub_rospy.msg import total



fx = 611.855712890625
fy = 611.8430786132812
cx = 317.46136474609375
cy = 247.88717651367188

stage=[]
cv_image=[]
depth_img=[]
color_img_update=0
depth_img_update=0
sig_info_update=0
args = []
model_xml=[]
model_bin=[]
ie=[]
net=[]
input_blob=[]
exec_net=[]
labels_map=[]
time_stamp=[]

logging.basicConfig(format="[ %(levelname)s ] %(message)s", level=logging.INFO, stream=sys.stdout)
log = logging.getLogger()


xml_path="/home/dd/inter-movidius-test/yolo-tiny/tensorflow-yolo-v3/frozen_darknet_yolov3_model.xml"


def build_argparser():
    parser = ArgumentParser(add_help=False)
    args = parser.add_argument_group('Options')
    args.add_argument('-h', '--help', action='help', default=SUPPRESS, help='Show this help message and exit.')
    args.add_argument("-m", "--model", help="Required. Path to an .xml file with a trained model.",
                      default=xml_path, type=str)
    args.add_argument("-i", "--input", help="Required. Path to a image/video file. (Specify 'cam' to work with "
                                            "camera)", required=False, type=str)
    args.add_argument("-l", "--cpu_extension",
                      help="Optional. Required for CPU custom layers. Absolute path to a shared library with "
                           "the kernels implementations.", type=str, default=None)
    args.add_argument("-d", "--device",
                      help="Optional. Specify the target device to infer on; CPU, GPU, FPGA, HDDL or MYRIAD is"
                           " acceptable. The sample will look for a suitable plugin for device specified. "
                           "Default value is CPU", default="MYRIAD", type=str)
    args.add_argument("--labels", help="Optional. Labels mapping file", default=None, type=str)
    args.add_argument("-t", "--prob_threshold", help="Optional. Probability threshold for detections filtering",
                      default=0.4, type=float)
    args.add_argument("-iout", "--iou_threshold", help="Optional. Intersection over union threshold for overlapping "
                                                       "detections filtering", default=0.4, type=float)
    args.add_argument("-ni", "--number_iter", help="Optional. Number of inference iterations", default=1, type=int)
    args.add_argument("-pc", "--perf_counts", help="Optional. Report performance counters", default=False,
                      action="store_true")
    args.add_argument("-r", "--raw_output_message", help="Optional. Output inference results raw values showing",
                      default=False, action="store_true")
    return parser


class YoloV3Params:
    # ------------------------------------------- Extracting layer parameters ------------------------------------------
    # Magic numbers are copied from yolo samples
    def __init__(self, param, side):
        self.num = 6 if 'num' not in param else int(param['num'])
        self.coords = 4 if 'coords' not in param else int(param['coords'])
        self.classes = 8 if 'classes' not in param else int(param['classes'])
        self.anchors = [10, 14, 23, 27, 37, 58, 81, 82, 135, 169, 344, 319] if 'anchors' not in param else [float(a) for a in param['anchors'].split(',')]

        if 'mask' in param:
            mask = [int(idx) for idx in param['mask'].split(',')]
            #print(mask)
            #mask=[[3, 4, 5], [0, 1, 2]]
            self.num = len(mask)

            maskedAnchors = []
            for idx in mask:
                maskedAnchors += [self.anchors[idx * 2], self.anchors[idx * 2 + 1]]
            self.anchors = maskedAnchors

        self.side = side


    def log_params(self):
        params_to_print = {'classes': self.classes, 'num': self.num, 'coords': self.coords, 'anchors': self.anchors}
        [log.info("         {:8}: {}".format(param_name, param)) for param_name, param in params_to_print.items()]


def entry_index(side, coord, classes, location, entry):
    side_power_2 = side ** 2
    n = location // side_power_2
    loc = location % side_power_2
    return int(side_power_2 * (n * (coord + classes + 1) + entry) + loc)


def scale_bbox(x, y, h, w, class_id, confidence, h_scale, w_scale):
    xmin = int((x - w / 2) * w_scale)
    ymin = int((y - h / 2) * h_scale)
    xmax = int(xmin + w * w_scale)
    ymax = int(ymin + h * h_scale)
    return dict(xmin=xmin, xmax=xmax, ymin=ymin, ymax=ymax, class_id=class_id, confidence=confidence)


def parse_yolo_region(blob, resized_image_shape, original_im_shape, params, threshold):
    # ------------------------------------------ Validating output parameters ------------------------------------------
    _, _, out_blob_h, out_blob_w = blob.shape
    assert out_blob_w == out_blob_h, "Invalid size of output blob. It sould be in NCHW layout and height should " \
                                     "be equal to width. Current height = {}, current width = {}" \
                                     "".format(out_blob_h, out_blob_w)

    # ------------------------------------------ Extracting layer parameters -------------------------------------------
    orig_im_h, orig_im_w = original_im_shape
    resized_image_h, resized_image_w = resized_image_shape
    objects = list()
    predictions = blob.flatten()
    side_square = params.side * params.side

    # ------------------------------------------- Parsing YOLO Region output -------------------------------------------
    for i in range(side_square):
        row = i // params.side
        col = i % params.side
        for n in range(params.num):
            obj_index = entry_index(params.side, params.coords, params.classes, n * side_square + i, params.coords)
            scale = predictions[obj_index]
            if scale < threshold:
                continue
            box_index = entry_index(params.side, params.coords, params.classes, n * side_square + i, 0)
            x = (col + predictions[box_index + 0 * side_square]) / params.side * resized_image_w
            y = (row + predictions[box_index + 1 * side_square]) / params.side * resized_image_h
            # Value for exp is very big number in some cases so following construction is using here
            try:
                w_exp = exp(predictions[box_index + 2 * side_square])
                h_exp = exp(predictions[box_index + 3 * side_square])
            except OverflowError:
                continue
            w = w_exp * params.anchors[2 * n]
            h = h_exp * params.anchors[2 * n + 1]
            for j in range(params.classes):
                class_index = entry_index(params.side, params.coords, params.classes, n * side_square + i,
                                          params.coords + 1 + j)
                confidence = scale * predictions[class_index]
                if confidence < threshold:
                    continue
                objects.append(scale_bbox(x=x, y=y, h=h, w=w, class_id=j, confidence=confidence,
                                          h_scale=orig_im_h / resized_image_h, w_scale=orig_im_w / resized_image_w))
    return objects


def intersection_over_union(box_1, box_2):
    width_of_overlap_area = min(box_1['xmax'], box_2['xmax']) - max(box_1['xmin'], box_2['xmin'])
    height_of_overlap_area = min(box_1['ymax'], box_2['ymax']) - max(box_1['ymin'], box_2['ymin'])
    if width_of_overlap_area < 0 or height_of_overlap_area < 0:
        area_of_overlap = 0
    else:
        area_of_overlap = width_of_overlap_area * height_of_overlap_area
    box_1_area = (box_1['ymax'] - box_1['ymin']) * (box_1['xmax'] - box_1['xmin'])
    box_2_area = (box_2['ymax'] - box_2['ymin']) * (box_2['xmax'] - box_2['xmin'])
    area_of_union = box_1_area + box_2_area - area_of_overlap
    if area_of_union == 0:
        return 0
    return area_of_overlap / area_of_union



def detect(current_total_dect,current_image,current_depth_img=[]):
    global fx,fy,cx,cy,stage,time_stamp
    (rows,cols,channels) = current_image.shape
    # cv2.imshow("Image window", current_image)
    #cv2.waitKey(3)
    
    getimg=current_image.copy()
    ##ori 16~44
    lower_blue=np.array([22,0,0])
    upper_blue=np.array([36,255,255])

    getimg = np.zeros([rows, cols, channels], np.uint8) 
    hsv = cv2.cvtColor(current_image, cv2.COLOR_BGR2HSV)

    mask = cv2.inRange(hsv, lower_blue, upper_blue)
    getimg=cv2.add(getimg,current_image,mask=mask)

    # # Read and pre-process input images
    n, c, h, w = net.inputs[input_blob].shape

    is_async_mode = False
    wait_key_code = 0

 
    cur_request_id = 0
    next_request_id = 1
    render_time = 0
    parsing_time = 0

    request_id = cur_request_id
    in_frame = cv2.resize(getimg, (w, h))

    # resize input_frame to network size
    in_frame = in_frame.transpose((2, 0, 1))  # Change data layout from HWC to CHW
    in_frame = in_frame.reshape((n, c, h, w))


    try:
        exec_net.start_async(request_id=request_id, inputs={input_blob: in_frame})
    except :
        info=pubmsg()
        info.label=88
        info.confidence=0
        info.x=0
        info.y=0
        info.z=0
        current_total_dect.result.append(info)
        rospy.loginfo("eddddddddddddddddddddddddddddddddddddddddddddddddddddddddd")
        return current_total_dect
    # Collecting object detection results
    objects = list()
    if exec_net.requests[cur_request_id].wait(-1) == 0:
        try:
            output = exec_net.requests[cur_request_id].outputs
        except:
            info=pubmsg()
            info.label=88
            info.confidence=0
            info.x=0
            info.y=0
            info.z=0
            current_total_dect.result.append(info)
            rospy.loginfo("eddddddddddddddddddddddddddddddddddddddddddddddddddddddddd")
            return current_total_dect

        # start_time = time()
        for layer_name, out_blob in output.items():
            layer_params = YoloV3Params(net.layers[layer_name].params, out_blob.shape[2])
            log.info("Layer {} parameters: ".format(layer_name))
            layer_params.log_params()
            objects += parse_yolo_region(out_blob, in_frame.shape[2:],
                                         getimg.shape[:-1], layer_params,
                                         args.prob_threshold)
        # parsing_time = time() - start_time

    # Filtering overlapping boxes with respect to the --iou_threshold CLI parameter
    objects = sorted(objects, key=lambda obj : obj['confidence'], reverse=True)
    for i in range(len(objects)):
        if objects[i]['confidence'] == 0:
            continue
        for j in range(i + 1, len(objects)):
            if intersection_over_union(objects[i], objects[j]) > args.iou_threshold:
                objects[j]['confidence'] = 0

    # Drawing objects with respect to the --prob_threshold CLI parameter
    objects = [obj for obj in objects if obj['confidence'] >= args.prob_threshold]

    if len(objects) and args.raw_output_message:
        log.info("\nDetected boxes for batch {}:".format(1))
        log.info(" Class ID | Confidence | XMIN | YMIN | XMAX | YMAX | COLOR ")

    origin_im_size = getimg.shape[:-1]
    #print(objects)

    for obj in objects:
        # Validation bbox of detected object
        if obj['xmax'] > origin_im_size[1] or obj['ymax'] > origin_im_size[0] or obj['xmin'] < 0 or obj['ymin'] < 0:
            continue
        color = (int(min(obj['class_id'] * 15, 255)),
                 min(obj['class_id'] * 20, 255), min(obj['class_id'] * 25+100, 255))
        det_label = labels_map[obj['class_id']] if labels_map and len(labels_map) >= obj['class_id'] else \
            str(obj['class_id'])

        if args.raw_output_message:
            log.info(
                "{:^9} | {:10f} | {:4} | {:4} | {:4} | {:4} | {} ".format(det_label, obj['confidence'], obj['xmin'],
                                                                          obj['ymin'], obj['xmax'], obj['ymax'],
                                                                          color))

        cv2.rectangle(getimg, (obj['xmin'], obj['ymin']), (obj['xmax'], obj['ymax']), color, 2)
        #rospy.loginfo("detect "+det_label)
        cv2.putText(getimg,
                    "#" + det_label + ' ' + str(round(obj['confidence'] * 100, 1)) + ' %',
                    (obj['xmin'], obj['ymin'] - 7), cv2.FONT_HERSHEY_COMPLEX, 0.6, color, 1)


        #pos_x,pos_y,pos_z=find_pos(obj['xmin'],obj['ymin'],obj['xmax'],obj['ymax'],current_depth_img)
        x_pos=0
        y_pos=0
        z_pos=0

        info=pubmsg()
        #info.label=0
        #info.confidence=0
        info.x=0
        info.y=0
        info.z=0
        info.label=obj['class_id']
        # rospy.loginfo("id==="+str(info.label))
        info.confidence=round(obj['confidence']*100,1)
        if stage==1 and len(current_depth_img)!=0 and (obj['xmax']-obj['xmin'])*(obj['ymax']-obj['ymin'])>=0:
            current_total_dect.header=time_stamp
            #get_depth_img=current_depth_img.copy()
            depth_box_width=obj['xmax']-obj['xmin']
            depth_box_height=obj['ymax']-obj['ymin']
            #print(depth_box_width)
            delta_rate=0.25
            x_box_min=int(obj['xmin']+depth_box_width*delta_rate)
            y_box_min=int(obj['ymin']+depth_box_height*delta_rate)
            x_box_max=int(obj['xmax']-depth_box_width*delta_rate)
            y_box_max=int(obj['ymax']-depth_box_height*delta_rate)
            #print(x_box_min)
            after_width=(depth_box_width*(1-2*delta_rate))
            after_height=(depth_box_height*(1-2*delta_rate))
            bb=current_depth_img[y_box_min:y_box_max,x_box_min:x_box_max]
            z_pos=bb.sum()/(after_width*after_height)*0.001
            #z_pos=sum(map(sum,bb))/(after_width*after_height)*0.001
            x_pos = (0.5 * (x_box_min + x_box_max) - cx) * z_pos / fx;
            y_pos = (0.5 * (y_box_min + y_box_max) - cy) * z_pos / fy;
            #print(x_pos)
            cv2.rectangle(getimg, (x_box_min,  y_box_min), (x_box_max,y_box_max), (0,255,255), 2)
            #"x="+str(x_pos)+" y="+str(y_pos)+
            info_pos=" z="+str(z_pos)
            cv2.putText(getimg,info_pos,(x_box_min,  y_box_min), cv2.FONT_HERSHEY_COMPLEX, 0.6, (0,255,0), 1)
            info.x=x_pos
            info.y=y_pos
            info.z=z_pos
        elif stage!=1 and (obj['xmax']-obj['xmin'])*(obj['ymax']-obj['ymin'])>=0:
            locate_x=(obj['xmax']+obj['xmin'])/2
            locate_y=(obj['ymax']+obj['ymin'])/2
            #print(depth_box_width)
            #print(getimg.shape)
            h,w,c=getimg.shape
            info.x=locate_x-w/2
            info.y=locate_y-h/2
            info.z=0
            cv2.putText(getimg,(str(round(info.x))+" "+str(round(info.y))),(int(locate_x),int(locate_y)), cv2.FONT_HERSHEY_COMPLEX, 0.6, (0,255,150), 1)
        else :
            info.confidence=0
        current_total_dect.result.append(info)
    # sort_obj=sorted(current_total_dect.result,key=lambda obj : current_total_dect.result['label'], reverse=True)
    cv2.imshow("DetectionResults", getimg)
    #cv2.imshow("current_depth_img", current_depth_img)
    cv2.waitKey(3)
    return current_total_dect


def load_model():
    global args,model_xml,model_bin,ie,net,input_blob,exec_net,labels_map
    args = build_argparser().parse_args()

    model_xml = args.model
    model_bin = os.path.splitext(model_xml)[0] + ".bin"

    # ------------- 1. Plugin initialization for specified device and load extensions library if specified -------------
    log.info("Creating Inference Engine...")
    ie = IECore()
    # if args.cpu_extension and 'CPU' in args.device:
    #     ie.add_extension(args.cpu_extension, "CPU")


    # -------------------- 2. Reading the IR generated by the Model Optimizer (.xml and .bin files) --------------------
    log.info("Loading network files:\n\t{}\n\t{}".format(model_xml, model_bin))
    net = IENetwork(model=model_xml, weights=model_bin)

    assert len(net.inputs.keys()) == 1, "Sample supports only YOLO V3 based single input topologies"

    # ---------------------------------------------- 4. Preparing inputs -----------------------------------------------
    log.info("Preparing inputs")
    input_blob = next(iter(net.inputs))

    #  Defaulf batch_size is 1
    net.batch_size = 1

    # Read and pre-process input images
    n, c, h, w = net.inputs[input_blob].shape

    if args.labels:
        with open(args.labels, 'r') as f:
            labels_map = [x.strip() for x in f]
    else:
        labels_map = None

    # ----------------------------------------- 5. Loading model to the plugin -----------------------------------------
    rospy.loginfo("Loading model to the plugin")
    exec_net = ie.load_network(network=net, num_requests=2, device_name=args.device)
    rospy.loginfo("Loaded model")


def doit():
        rate = rospy.Rate(30)
        global cv_image, color_img_update,depth_img,depth_img_update,stage,sig_info_update
        # rospy.loginfo("Lo the VideoCapture")
        # capture = cv2.VideoCapture(9)
        # rospy.loginfo("9 the VideoCapture")
        pub_total=rospy.Publisher("/total_info",total,queue_size = 1)
        get_total_update=0
        local_flag=0
        capture=[]
        open_down_cam=0
        while not rospy.is_shutdown():
            get_total=total()
            total_dect=total()
############################
            # if color_img_update == 1 and depth_img_update==1 : #and sig_info_update==1
            if stage!=1:
                stage=1
            # if stage==100:
            #     if(local_flag==0):
            #         local_flag=1
            #         for i in range(5):
            #             et, frame = capture.read()
            #     else:
            #         et, frame = capture.read()
            #     if et:
            #         if len(frame)!=0:
                       
            #             try:
            #                  get_total=detect(total_dect,frame)
            #             except:
            #                 info=pubmsg()
            #                 info.label=88
            #                 info.confidence=0
            #                 info.x=0
            #                 info.y=0
            #                 info.z=0
            #                 get_total.result.append(info)
            #                 rospy.loginfo("stage--0   dddddddddddddddddddddddddddddddddddddddddddddd")
            #             del frame
            #             get_total_update=1
            #             et=False

            if stage==1 and color_img_update == 1 and depth_img_update==1:
                if len(cv_image)!=0 and len(depth_img)!=0:
                    try:
                        get_total=detect(total_dect,cv_image,depth_img)
                    except:
                        info=pubmsg()
                        info.label=88
                        info.confidence=0
                        info.x=0
                        info.y=0
                        info.z=0
                        get_total.result.append(info)
                        rospy.loginfo("stage--1   dddddddddddddddddddddddddddddddddddddddddddddd")
                    get_total_update=1
                    color_img_update = 0
                    depth_img_update =0
                    local_flag=0
            if stage==2:
                info=pubmsg()
                info.label=100
                info.confidence=0
                info.x=0
                info.y=0
                info.z=0
                get_total.result.append(info)
                get_total_update=1
            if stage==3 :
                if open_down_cam==0:
                    rospy.loginfo("open camera")
                    time.sleep(3)
                    capture = cv2.VideoCapture("/dev/video20")
                    open_down_cam=1
                    rospy.loginfo("camera opened success")
                if(local_flag==0):
                    local_flag=1
                    for i in range(5):
                        et, frame = capture.read()
                else:
                    et, frame = capture.read()
                if et:
                    if len(frame)!=0:
                        try:
                             get_total=detect(total_dect,frame)
                        except:
                            info=pubmsg()
                            info.label=88
                            info.confidence=0
                            info.x=0
                            info.y=0
                            info.z=0
                            get_total.result.append(info)
                            rospy.loginfo("stage--3   dddddddddddddddddddddddddddddddddddddddddddddd")
                        del frame
                        get_total_update=1
                        et=False
            if  get_total_update==1:
                get_total_update=0
                # if len(get_total)==0:
                #         info1=pubmsg()
                #         info1.label=30
                #         info1.confidence=0
                #         info1.x=0
                #         info1.y=0
                #         info1.z=0
                #         get_total_now.result.append(info1)
                pub_total.publish(get_total)
            rate.sleep()





class process:
    def __init__(self):
        self.depth_img=None
        self.color_img=None
        self.sig_info=None
    def sig_info_callback(self,msg):
        global stage,sig_info_update
        self.sig_info=msg.data
        stage=self.sig_info
        sig_info_update=1
        rospy.loginfo(stage)

    def color_callback(self,msg):
        global cv_image,color_img_update
        bridge = CvBridge()
        self.color_img = bridge.imgmsg_to_cv2(msg, "bgr8")
        cv_image=self.color_img.copy()
        color_img_update=1

    def depth_callback(self,msg):
        global depth_img,depth_img_update,time_stamp
        bridge = CvBridge()
        self.depth_img=bridge.imgmsg_to_cv2(msg, "32FC1")
        depth_img=self.depth_img.copy()
        time_stamp=msg.header
        depth_img_update=1


def my_listener():
    rospy.init_node('depth_box', anonymous=False,log_level=rospy.INFO)
    dododo=process()

    sig_info=rospy.Subscriber("/sig_info",UInt8,dododo.sig_info_callback)
    image_sub = rospy.Subscriber("/camera/color/image_raw",Image,dododo.color_callback)
    image_depth = rospy.Subscriber("/camera/aligned_depth_to_color/image_raw",Image,dododo.depth_callback)
    try:
        load_model()
    except :
        # rospy.loginfo("load model failed,please restart !!!")
        rate = rospy.Rate(1)
        pub_total=rospy.Publisher("/load_model",String,queue_size = 1)
        while not rospy.is_shutdown():
            rospy.loginfo("load failed!!!")
            pub_total.publish("load model failed,please restart !!!")
            rate.sleep()
        return
    doit()


if __name__ == '__main__':
    my_listener()

