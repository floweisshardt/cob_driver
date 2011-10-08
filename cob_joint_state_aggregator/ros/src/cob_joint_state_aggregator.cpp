#include "ros/ros.h"
#include "sensor_msgs/JointState.h"


// the joint state of one joint
struct JointState {
	double position;
	double velocity;
	double effort;
};


typedef std::map<std::string, JointState> MapType;
MapType joints;


void jsCallback(const sensor_msgs::JointState::ConstPtr& msg)
{
	MapType::iterator iter = joints.begin();
	for(unsigned int i = 0; i < msg->name.size(); i++)
	{
		//std::cout << msg->name[i] << std::endl;
		
		iter = joints.find(msg->name[i]);
		
		if (iter != joints.end())
		{
			iter->second.position = msg->position[i];
			iter->second.velocity = msg->velocity[i];
			iter->second.effort = msg->effort[i];
		}
	}
	//ROS_INFO("updated joint_states");
}

sensor_msgs::JointState getJointStatesMessage()
{
	sensor_msgs::JointState msg;
	msg.header.stamp = ros::Time::now();
	msg.name.resize(joints.size());
	msg.position.resize(joints.size());
	msg.velocity.resize(joints.size());
	msg.effort.resize(joints.size());


	MapType::const_iterator end = joints.end();
	MapType::iterator iter = joints.begin();
	int count = 0; 
	for (MapType::const_iterator it = joints.begin(); it != end; ++it)
	{
		msg.name[count] = it->first;
		msg.position[count] = it->second.position;
		msg.velocity[count] = it->second.velocity;
		msg.effort[count] = it->second.effort;
		
		count++;
	}
	return msg;
}

int main(int argc, char **argv)
{	
	ros::init(argc, argv, "cob_joint_state_aggregator");
	ros::NodeHandle n;
	
	ros::Subscriber sub = n.subscribe("/joint_states", 50, jsCallback);
	ros::Publisher topicPub_JointState_ = n.advertise<sensor_msgs::JointState>("/joint_states_combined", 1);

	XmlRpc::XmlRpcValue JointName_param_;
	std::string JointName;
	if (n.hasParam("joint_names"))
	{
		n.getParam("joint_names", JointName_param_);
	}
	else
	{
		ROS_ERROR("Parameter joint_names not set");
	}
	for (int i = 0; i<JointName_param_.size(); i++ )
	{
		JointName = (std::string)JointName_param_[i];
		ROS_DEBUG("Inserting joint %s",JointName.c_str());
		
		JointState state;
		state.position = 0.0;
		state.velocity = 0.0;
		state.effort = 0.0;

		joints[JointName] = state;
	}

	ros::Rate loop_rate(100);
	while (ros::ok())
	{  
		sensor_msgs::JointState msg = getJointStatesMessage();
		topicPub_JointState_.publish(msg);
		ros::spinOnce();
	 	loop_rate.sleep();
	}

	return 0;
}
