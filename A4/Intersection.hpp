#pragma once
#include <glm/glm.hpp>
#include <math.h>

class GeometryNode;
class Triangle;

struct Intersection {
	bool has_intersected;
	double t; // the value to plug into parametric
	GeometryNode *node; // what was hit
	glm::mat4 invtrans;
	glm::vec3 local_intersection;
	Triangle *tri; // If a mesh was hit

	Intersection(): has_intersected(false), t(nan("")), node(NULL), invtrans(glm::mat4()), local_intersection(glm::vec3()), tri(NULL) {}

	Intersection(bool has_intersected, double t, GeometryNode *node)
		: has_intersected(has_intersected), t(t), node(node), invtrans(glm::mat4()), local_intersection(glm::vec3()),tri(NULL) {}

	Intersection(double t, GeometryNode *node): t(t), node(node), invtrans(glm::mat4()), local_intersection(glm::vec3()),tri(NULL) {
		has_intersected = isnan(t) ? false : true;
	}
	Intersection(double t): t(t), node(NULL), invtrans(glm::mat4()), tri(NULL), local_intersection(glm::vec3()){
		has_intersected = isnan(t) ? false : true;
	}
	~Intersection() {}
};
