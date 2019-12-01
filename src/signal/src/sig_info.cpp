#include <ros/ros.h>
#include "std_msgs/UInt8.h"
int main(int argc,char **argv){
	ros::init(argc,argv,"sin_info");
	ros::NodeHandle n;
	ros::Publisher  sig_pub=n.advertise<std_msgs::UInt8>("sig_info",100);
	ros::Rate loop_rate(50);
	int dd=0;
	while (ros::ok()){
		std_msgs::UInt8 info;

		if(dd<500){
			info.data=0;
		}
		if(500<=dd&&dd<1200){
			info.data=1;
		}
		if(1200<=dd&&dd<1400){
			info.data=2;
		}
		if(1400<=dd&&dd<2000){
			info.data=3;
		}
		else if(dd>=2000){
			dd=0;
		}
		ROS_INFO_STREAM(info);
		ROS_INFO_STREAM(dd);
		dd=dd+1;
		sig_pub.publish(info);
		ros::spinOnce();
		loop_rate.sleep();

	}
	return 0;
}
