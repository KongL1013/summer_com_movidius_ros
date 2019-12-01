#include <ros/ros.h>
#include <pub_rospy/pubmsg.h>
#include <pub_rospy/total.h>
void callback(const pub_rospy::total::ConstPtr &msg)
{
	ROS_INFO("i get");
	//int label=msg->label;
	//ROS_INFO(msg.label);

}

int main(int argc,char **argv){
	ros::init(argc,argv,"get_info");
	ros::NodeHandle n;
	ros::Subscriber sub=n.subscribe("/total_info",100,callback);
	ros::spin();
	return 0;
}