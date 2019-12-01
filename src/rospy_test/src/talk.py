#!/usr/bin/env python3.7

import rospy
from std_msgs.msg import String
import cv2
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError

def callback(self,data):
    #rospy.loginfo(rospy.get_caller_id() + 'I heard %s', data.data)
    try:
        cv_image = self.bridge.imgmsg_to_cv2(data, "bgr8")
    except CvBridgeError as e:
        print("miss")
    cv2.imshow("dd",cv_image)

    cv2.waitKey(3)


class image_converter:
 
  def __init__(self):
    #self.image_pub = rospy.Publisher("image_topic_2",Image)
 
    self.bridge = CvBridge()
    self.image_sub = rospy.Subscriber("/camera/color/image_raw",Image,self.callback)
 
  def callback(self,data):
    try:
      cv_image = self.bridge.imgmsg_to_cv2(data, "bgr8")
    except CvBridgeError as e:
      print(e)
 
    # (rows,cols,channels) = cv_image.shape
    # if cols > 60 and rows > 60 :
    #   cv2.circle(cv_image, (50,50), 10, 255)
 
    cv2.imshow("Image window", cv_image)
    cv2.waitKey(3)
 
    # try:
    #   self.image_pub.publish(self.bridge.cv2_to_imgmsg(cv_image, "bgr8"))
    # except CvBridgeError as e:
    #   print(e)

def listener():

    # In ROS, nodes are uniquely named. If two nodes with the same
    # name are launched, the previous one is kicked off. The
    # anonymous=True flag means that rospy will choose a unique
    # name for our 'listener' node so that multiple listeners can
    # run simultaneously.
    # rospy.init_node('listener', anonymous=True)

    # rospy.Subscriber('/camera/color/image_raw', Image, callback)

    ic = image_converter()
    rospy.init_node('image_converter', anonymous=True)
    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    listener()

