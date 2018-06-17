#pragma once

#include <glm/glm.hpp>
#include <math.h>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"

#include "GeometryNode.hpp"

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
);

struct Intersection {
	bool has_intersected;
	double t; // the value to plug into parametric
	GeometryNode *node; // what was hit

	Intersection(): has_intersected(false), t(nan("")), node(NULL) {}

	Intersection(bool has_intersected, double t, GeometryNode *node)
		: has_intersected(has_intersected), t(t), node(node) {}

	Intersection(double t, GeometryNode *node): t(t), node(node) {
		has_intersected = isnan(t) ? false : true;
	}
};
