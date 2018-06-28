#pragma once
#include <glm/glm.hpp>
#include <math.h>

#include "Triangle.hpp"

class GeometryNode;

struct Intersection {
	bool has_intersected;
	double t; // the value to plug into parametric
	GeometryNode *node; // what was hit
	glm::mat4 invtrans;
	glm::vec3 local_intersection;
	Triangle *tri; // If a mesh was hit

	Intersection();
	Intersection(double t);
	Intersection(double t, GeometryNode *node);
	~Intersection();
};
