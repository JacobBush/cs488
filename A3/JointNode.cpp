#include "JointNode.hpp"
//#include <iostream>
//using namespace std;

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name), picked(false)
{
	m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;

	current_jointx = m_joint_x.init;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;

	current_jointy = m_joint_y.init;
}

void JointNode::updateJointBy(float amount) {
	//cout << "updating joint with value " << current_jointx << ", " << current_jointy;
	//cout << "by amount " << amount << endl;
	//cout << "the joint has xmax" << m_joint_x.max << ", xmin " << m_joint_x.min << endl;
	//cout << "the joint has ymax" << m_joint_y.max << ", ymin " << m_joint_y.min << endl;

	if (!picked) return;

	current_jointx += (double)amount;
	if (current_jointx > m_joint_x.max) {
		current_jointx = m_joint_x.max;
	}
	if (current_jointx < m_joint_x.min) {
		current_jointx = m_joint_x.min;
	}

	current_jointy += (double)amount;
	if (current_jointy > m_joint_y.max) {
		current_jointy = m_joint_y.max;
	}
	if (current_jointy < m_joint_y.min) {
		current_jointy = m_joint_y.min;
	}
	//cout << "updated joint to value " << current_jointx << ", " << current_jointy;
}
