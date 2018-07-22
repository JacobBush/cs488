#pragma once
#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "Intersection.hpp"
#include <vector>

// Implementation of Octree for acceleration
class SpacePartition {
public:
	SpacePartition();
	~SpacePartition();

	Intersection *intersect(glm::vec3 a, glm::vec3 b);
	void initialize(SceneNode *root);

private:
	SpacePartition(glm::vec3 posn, double size, std::vector<GeometryNode *> parent_nodes, uint depth);
	glm::vec3 get_bot_left_corner(SceneNode * node);
	glm::vec3 get_top_right_corner(SceneNode * node);

	void partition();
	void fill_root();
	void fill(std::vector<GeometryNode *> parent_nodes);
	void recursive_fill_root(SceneNode * node);
	bool box_intersect(glm::vec3 b1, glm::vec3 t1, glm::vec3 b2, glm::vec3 t2);

	glm::vec3 ray_point_at_parameter(const glm::vec3 & a, const glm::vec3 & b, double t);

	Intersection *local_intersect(glm::vec3 a, glm::vec3 b, GeometryNode *node);
	Intersection * bb_intersect(glm::vec3 a, glm::vec3 b);

	static const int MAX_NODES = 50;

	SpacePartition *children[8]; // 8 for Octree
	std::vector<GeometryNode *> nodes; 

	SceneNode * root;
	glm::vec3 posn; // bot left corner
	double size; // side length
	bool is_partitioned;
	const uint depth;
};
