#include "Primitive.hpp"
#include <iostream>
#include <math.h>
#include <glm/ext.hpp>

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

Cube::~Cube()
{
}

NonhierSphere::~NonhierSphere()
{
}

NonhierBox::~NonhierBox()
{
}

glm::vec3 Primitive::ray_point_at_parameter(const glm::vec3 &a, const glm::vec3 &b, double t) {
	return a + t*(b-a);
}

double Primitive::plane_intersection(glm::vec3 p0, glm::vec3 N, glm::vec3 a, glm::vec3 b) {
	double den = glm::dot(N, (b-a));
	if (glm::abs(den) >= EPSILON) {
		return glm::dot(N, (p0-a)) / den;
	} else {
		return nan("");
	}
}

double Primitive::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	// -1 -> no intersection
	return nan("");
}

double Sphere::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	// -1 -> no intersection

	// m_pos = [0,0,0]
	// m_radius = 1

	double A = glm::dot(b-a, b-a);
	double B = 2.0 * glm::dot(b-a, a);
	double C = glm::dot(a, a) - 1;

	double discriminant = B*B - 4.0*A*C;

	//std::cout << "discriminant: " << discriminant << std::endl;

	if (discriminant < -EPSILON) {
		// negative - no roots
		return nan("");
	} else if (glm::abs(discriminant) <= EPSILON) {
		// approximately 0 - 1 root
		return -2.0*C/B;
	} else {
		// 2 roots
		double sqrt_disc = glm::sqrt(discriminant);
		double t1 = -2.0*C/(B + sqrt_disc);
		double t2 = -2.0*C/(B - sqrt_disc);

		return glm::min(t1,t2);
	}
}

double Cube::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	double t = nan("");
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			double tprime = intersect_side(i, j, a, b);
			if (isnan(t) || (!isnan(tprime) && tprime < t)) t = tprime;
		}
	}
	return t;
}

double NonhierSphere::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	// -1 -> no intersection
	//std::cout << "in NonhierSphere::intersection" << std::endl;

	double A = glm::dot(b-a, b-a);
	double B = 2.0 * glm::dot(b-a, a-m_pos);
	double C = glm::dot(a-m_pos, a-m_pos) - m_radius*m_radius;

	double discriminant = B*B - 4.0*A*C;

	//std::cout << "discriminant: " << discriminant << std::endl;

	if (discriminant < -EPSILON) {
		// negative - no roots
		return nan("");
	} else if (glm::abs(discriminant) <= EPSILON) {
		// approximately 0 - 1 root
		return -2.0*C/B;
	} else {
		// 2 roots
		double sqrt_disc = glm::sqrt(discriminant);
		double t1 = -2.0*C/(B + sqrt_disc);
		double t2 = -2.0*C/(B - sqrt_disc);

		return glm::min(t1,t2);
	}
}

bool Cube::point_on_side(glm::vec3 p, uint side, bool front) {
	double sign = front ? 1.0 : -1.0;
	glm::vec3 center = glm::vec3(0.5);

	if (glm::abs(p[side] - 0.5 - sign*0.5) > EPSILON) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - 0.5) > 0.5 + EPSILON) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - 0.5) > 0.5 + EPSILON) {return false;}
	return true;
}

double Cube::intersect_side(uint side, bool front, glm::vec3 a, glm::vec3 b) {
	double sign = front ? 1.0 : -1.0;

	glm::vec3 p0 = glm::vec3(0.5);
	glm::vec3 N = glm::vec3(0.0,0.0,0.0);
	p0[side] += sign*0.5;
	N[side] = sign;

	double t = plane_intersection(p0, N, a, b);
	if (isnan(t)) return nan("");
	glm::vec3 p = ray_point_at_parameter(a,b,t);
	if (!point_on_side(p, side, front)) {
		return nan("");
	}

	return t;
}

bool NonhierBox::point_on_side(glm::vec3 p, uint side, bool front) {
	double sign = front ? 1.0 : -1.0;
	glm::vec3 center = m_pos + glm::vec3(m_size/2.0);

	if (glm::abs(p[side] - center[side] - sign*m_size/2.0) > EPSILON) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - center[side]) > m_size/2.0 + EPSILON) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - center[side]) > m_size/2.0 + EPSILON) {return false;}
	return true;
}

double NonhierBox::intersect_side(uint side, bool front, glm::vec3 a, glm::vec3 b) {
	double sign = front ? 1.0 : -1.0;

	glm::vec3 p0 = m_pos + glm::vec3(m_size/2.0);
	glm::vec3 N = glm::vec3(0.0,0.0,0.0);
	p0[side] += sign*m_size/2.0;
	N[side] = sign;

	double t = plane_intersection(p0, N, a, b);
	if (isnan(t)) return nan("");
	glm::vec3 p = ray_point_at_parameter(a,b,t);
	if (!point_on_side(p, side, front)) {
		return nan("");
	}

	return t;
}


double NonhierBox::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	double t = nan("");
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			double tprime = intersect_side(i, j, a, b);
			if (isnan(t) || (!isnan(tprime) && tprime < t)) t = tprime;
		}
	}
	return t;
}

glm::vec3 Primitive::get_normal_at_point(glm::vec3 p) {
	return glm::vec3(0.0,0.0,0.0);
}

glm::vec3 Sphere::get_normal_at_point(glm::vec3 p) {
	return glm::normalize(p);
}

glm::vec3 Cube::get_normal_at_point(glm::vec3 p) {
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			if (point_on_side(p, i, j)) {
				glm::vec3 N = glm::vec3(0.0,0.0,0.0);
				N[i] = (double)j;
				return N;
			}
		}
	}
}

glm::vec3 NonhierSphere::get_normal_at_point(glm::vec3 p) {
	return glm::normalize(p - m_pos);
}

glm::vec3 NonhierBox::get_normal_at_point(glm::vec3 p) {
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			if (point_on_side(p, i, j)) {
				glm::vec3 N = glm::vec3(0.0,0.0,0.0);
				N[i] = (double)j;
				return N;
			}
		}
	}
}
