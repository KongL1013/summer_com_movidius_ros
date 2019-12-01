#!/usr/bin/env python
# Software License Agreement (BSD License)
#
# Copyright (c) 2008, Willow Garage, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Revision $Id$

## Simple talker demo that published std_msgs/Strings messages
## to the 'chatter' topic

import rospy
import os
import time
from std_msgs.msg import String,UInt8
from pub_rospy.msg import pubmsg
from pub_rospy.msg import total
from sensor_msgs.msg import CameraInfo

global pynodeAliveTime, rsAliveTime, state

def pynodecallback(msg): #for node: /depth_box
    global pynodeAliveTime
    pynodeAliveTime = rospy.Time.now()

def rscallback(msg):    #for node: /camera/realsense2_camera_manager
    global rsAliveTime
    rsAliveTime = rospy.Time.now()

def statecallback(msg):
    global state
    state = msg.data % 10

def restartRS():
    rospy.logerr('Killing Realsense...')
    os.system('rosnode kill /camera/realsense2_camera_manager')
    rospy.logerr('Restarting Realsense...')
    os.system('roslaunch realsense2_camera com.launch &')
    time.sleep(10)
    rospy.logerr('Realsense restarted.')

def restartPynode():
    rospy.logerr('Killing pythonNode...')
    os.system('rosnode kill /depth_box')
    rospy.logerr('Restarting pythonNode...')
    os.system('python2.7 lsj_src/pycode/rs.py &')
    time.sleep(5)
    rospy.logerr('PythonNode restarted.')





def node_holder():
    global pynodeAliveTime, rsAliveTime, state
    rospy.init_node('node_holder', anonymous=True)

    rospy.Subscriber('/total_info', total, pynodecallback)
    rospy.Subscriber('/camera/color/camera_info', CameraInfo, rscallback)
    rospy.Subscriber('/sig_info', UInt8, statecallback)

    pynodeAliveTime = rospy.Time.now()
    rsAliveTime = rospy.Time.now()
    state = 0


    rate = rospy.Rate(30)
    while not rospy.is_shutdown():
        # hello_str = "hello world %s" % rospy.get_time()
        # rospy.loginfo(hello_str)
        # pub.publish(hello_str)
        pynodeNoResponseTime = rospy.Time.now() - pynodeAliveTime
        rsNoResponseTime = rospy.Time.now() - rsAliveTime

        if state == 1:
            if rsNoResponseTime.to_sec() > 1:
                restartRS()
                restartPynode()
            elif pynodeNoResponseTime.to_sec() > 1:
                restartPynode()
        elif state == 2:
            if rsNoResponseTime.to_sec() > 1:
                restartRS()


        rate.sleep()

if __name__ == '__main__':
    try:
        node_holder()
    except rospy.ROSInterruptException:
        pass
