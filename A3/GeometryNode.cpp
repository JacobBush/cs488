#include "GeometryNode.hpp"
#include "JointNode.hpp"

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
		const std::string & meshId,
		const std::string & name
)
	: SceneNode(name),
	  meshId(meshId)
{
	m_nodeType = NodeType::GeometryNode;
}

bool GeometryNode::parentJointIsSelected () {
	if (parent->m_nodeType != NodeType::JointNode) return false;
	if (!((JointNode *)parent)->picked) return false;
	return true;
}