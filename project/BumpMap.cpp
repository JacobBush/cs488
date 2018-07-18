#include "BumpMap.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include "math.h"

BumpMap::~BumpMap() {}
BumpMap::BumpMap(const char* filename) : ObjectMap(filename) {}

/*
 * x,y need to be in [0.0,1.0]
 * perturnbed [0,1,0] vector, which can be translated to normal position
 */
glm::vec3 BumpMap::get_perturbed_normal_at_point(double x, double y) {
	if (x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0) {
		std::cout << "Call to BumpMap::get_permutation_at_point(double x, double y) with parameters" << std::endl;
		std::cout << "x: " << x << ", y: " << y << "while they must be in [0.0, 1.0]" << std::endl;
		return glm::vec3(0.0,0.0,0.0);
	}

	uint a = round(x * (width - 1));
	uint b = round(y * (height - 1));

	// color at point and its neighbours
    glm::vec4 P = get_value_at_point(a , b);
	glm::vec4 U = (a == (width - 1)) ? get_value_at_point(a - 1, b) : get_value_at_point(a + 1, b);
	glm::vec4 V = (b == (height - 1)) ? get_value_at_point(a, b - 1) : get_value_at_point(a, b + 1);

	// convert all 3 points to grayscale by average -- ignore alpha (height of point)
	double Hp = (P.x + P.y + P.z)/3.0;
	double Hu = (U.x + U.y + U.z)/3.0;
	double Hv = (V.x + V.y + V.z)/3.0;

	// calculate slopes
	double Du = (a == (width - 1)) ? Hp-Hu : Hu-Hp;
	double Dv = (b == (height - 1)) ? Hp-Hv : Hv-Hp;

	// new vector is perturbed in those directions
	return glm::vec3(Du, 1.0, Dv);
}
