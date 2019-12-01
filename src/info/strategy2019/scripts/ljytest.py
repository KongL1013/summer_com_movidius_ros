#!/usr/bin/env python2.7
import rospy
from std_msgs.msg import Int32
from pub_rospy.msg import pubmsg
from pub_rospy.msg import total
import time 


def talker():
	
	rospy.init_node("sender",anonymous=True)
	pub_total=rospy.Publisher("/sig_info",total,queue_size = 10)
	rate=rospy.Rate(5)
	a=0
	while not rospy.is_shutdown():
		if a<10 :
			info=pubmsg()
			totaldd=total()
			info.label=11
			info.confidence=0.88
			info.x=11.256
			info.y=21.257
			info.z=31.259
			for i in range(3):
				totaldd.result.append(info)
			rospy.loginfo(totaldd)
			#print(type(totaldd))
			rospy.loginfo(len(totaldd.result))
			pub_total.publish(totaldd)
			rate.sleep()
			a=a+1
		else :
			time.sleep(20)
if __name__=='__main__':
	try:
		talker()
	except rospy.ROSInterruptException:
		pass
