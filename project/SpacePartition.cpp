#include "SpacePartition.hpp"

SpacePartition::SpacePartition() {
	for (uint i = 0; i < 8; i++) children[i] = NULL;
	nodes = std::vector<SceneNode *>();
}

SpacePartition::~SpacePartition() {
	for (uint i = 0; i < 8; i++) { // recursively delete each child
		if (children[i] != NULL) {
			delete children[i];
			children[i] = NULL;
		}
	}
}

void SpacePartition::initialize(SceneNode *root) {
	
}
