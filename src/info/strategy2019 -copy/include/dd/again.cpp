// #include <ros/ros.h>
// #include <pub_rospy/pubmsg.h>

// #include "std_msgs/String.h"
// void callback(const pub_rospy::pubmsg::ConstPtr &msg){
// 	ROS_INFO("i get");
// 	int label=msg->label;
// 	ROS_INFO_STREAM(label);

// }

// int main(int argc,char **argv){
// 	ros::init(argc,argv,"get_info");
// 	ros::NodeHandle n;
// 	ros::Subscriber sub=n.subscribe("/detect_info",100,callback);
// 	ros::spin();
// 	return 0;
// }

#include <ros/ros.h>
#include <pub_rospy/pubmsg.h>
#include <pub_rospy/total.h>
void callback(const pub_rospy::total::ConstPtr &msg)
{
	//ROS_INFO("i get");

	pub_rospy::pubmsg data=msg->result[0];
	//int label=msg->label;
	//ROS_INFO_STREAM(msg->result.size());
	for(int i=0;i<msg->result.size();i++){
		//ROS_INFO_STREAM(i);
		ROS_INFO_STREAM(msg->result[i].label);
		ROS_INFO_STREAM(msg->result[i].x);
	}

}

int main(int argc,char **argv){
	ros::init(argc,argv,"get_info");
	ros::NodeHandle n;
	ros::Subscriber sub=n.subscribe("/total_info",100,callback);
	ros::spin();
	return 0;
}