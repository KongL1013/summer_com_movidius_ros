#include <ros/ros.h>
#include <pub_rospy/pubmsg.h>

//#include "std_msgs/String.h"
void callback(const pub_rospy::pubmsg::ConstPtr &msg){
	ROS_INFO("i get");
	std::vector<int> label=msg.label->data;
	ROS_INFO(msg.label);

}

int main(int argc,char **argv){
	ros::init(argc,argv,"get_info");
	ros::NodeHandle n;
	ros::Subscriber sub=n.subscribe("/detect_info",100,callback);
	ros::spin();
	return 0;
}
