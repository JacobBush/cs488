#pragma once

#include <glm/glm.hpp>
#include <math.h>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"

#include "GeometryNode.hpp"
#include "Intersection.hpp"

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

glm::vec3 get_color_of_intersection(Intersection *intersection, glm::vec3 a, glm::vec3 b,
								    const glm::vec3 & ambient, const std::list<Light *> & lights,
								    uint hits_allowed, SceneNode * node);
