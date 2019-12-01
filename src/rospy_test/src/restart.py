#!/usr/bin/env python2.7


import os
import time
import rospy
from std_msgs.msg import String,UInt8
from pub_rospy.msg import total




global 

class process:
    def __init__(self):
        self.depth_img=None
        self.color_img=None
        self.sig_info=None
    def listen_info_callback(self,msg):
        global 
        self.sig_info=msg.data
        stage=self.sig_info
        sig_info_update=1
        rospy.loginfo(" sig_info_update plugin")


def my_listener():
    rospy.init_node('/listen_depth_box', anonymous=True,log_level=rospy.INFO)
    listen=rospy.Subscriber("/total_info",total,dododo.listen_info_callback)
    dododo=process()

    os.system("rosnode kill /depth_box")
    os.system("python2.7 sth")







if __name__ == '__main__':
    my_listener()