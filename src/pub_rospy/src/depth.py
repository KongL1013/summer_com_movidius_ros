#!/usr/bin/env python2.7
import rospy
from std_msgs.msg import String
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError
import cv2
import numpy as np





fx = 611.855712890625
fy = 611.8430786132812
cx = 317.46136474609375
cy = 247.88717651367188

color_img_update=0
depth_img_update=0
cv_image=[]
depth_img=[]


def doit():
		rate = rospy.Rate(100)
		global cv_image, color_img_update,depth_img,depth_img_update
		while not rospy.is_shutdown():
			if color_img_update == 1 and depth_img_update==1:
				box=[300,220,340,260]
				bb=depth_img[200:280,280:360]
				# bb=depth_img[200:280:1]
				# bb.extend(depth_img[280:360:1])
				#kk=dd[300:340:1]
				print(bb.shape)
				dsum=sum(map(sum,bb))/(80*80)/1000
				print(dsum)

				cv2.rectangle(cv_image, (280, 200), (360,280), (0,255,255), 2)
				cv2.putText(cv_image,str(dsum),(320, 240), cv2.FONT_HERSHEY_COMPLEX, 0.6, (0,255,0), 1)
				cv2.imshow("ggggg",cv_image)
				cv2.waitKey(10)
				color_img_update = 0
				depth_img_update =0
			rate.sleep()



class process:
	def __init__(self):
		self.depth_img=None
		self.color_img=None
	def color_callback(self,msg):
		global cv_image,color_img_update
		bridge = CvBridge()
		self.color_img = bridge.imgmsg_to_cv2(msg, "bgr8")
		cv_image=self.color_img.copy()
		color_img_update=1

		# cv2.imshow("ggggg",self.color_img)
		# cv2.waitKey(10)
	def depth_callback(self,msg):
		global depth_img,depth_img_update
		bridge = CvBridge()
		self.depth_img=bridge.imgmsg_to_cv2(msg, "32FC1")
		depth_img=self.depth_img.copy()
		depth_img_update=1

		h,w=self.depth_img.shape
		#print(str(h)+" "+str(w))  #(480,640)

		#print(self.depth_img[200])
		#cv_image_array = np.array(self.depth_img, dtype = np.dtype('f8'))




		#cv2.imshow("Image from my node", cv_image_array)
		# cv2.waitKey(10)

def my_listener():
    rospy.init_node('depth_box', anonymous=True)
    dododo=process()

    image_sub = rospy.Subscriber("/camera/color/image_raw",Image,dododo.color_callback)
    image_depth = rospy.Subscriber("/camera/aligned_depth_to_color/image_raw",Image,dododo.depth_callback)
    doit()
    #rospy.spin()
    #doit()


if __name__ == '__main__':
    my_listener()