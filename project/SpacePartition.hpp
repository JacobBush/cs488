#pragma once
#include "SceneNode.hpp"
#include <vector>

// Implementation of Octree for acceleration
class SpacePartition {
public:
	SpacePartition();
	~SpacePartition();

	void initialize(SceneNode *root);

private:
	SpacePartition *children[8]; // 8 for Octree
	std::vector<SceneNode *> nodes; 
};
