#include "Primitive.hpp"
#include <iostream>
#include <math.h>

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

double Primitive::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	// -1 -> no intersection
	return nan("");
}

double Sphere::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	// -1 -> no intersection
	return nan("");
}

double Cube::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	// -1 -> no intersection
	return nan("");
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
	} else if (discriminant <= EPSILON && discriminant >= -EPSILON) {
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

double NonhierBox::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	// -1 -> no intersection
	return nan("");
}

glm::vec3 Primitive::get_normal_at_point(glm::vec3 p) {
	return glm::vec3(0.0,0.0,0.0);
}

glm::vec3 Sphere::get_normal_at_point(glm::vec3 p) {
	return glm::vec3(0.0,0.0,0.0);
}

glm::vec3 Cube::get_normal_at_point(glm::vec3 p) {
	return glm::vec3(0.0,0.0,0.0);
}

glm::vec3 NonhierSphere::get_normal_at_point(glm::vec3 p) {
	return glm::normalize(p - m_pos);
}

glm::vec3 NonhierBox::get_normal_at_point(glm::vec3 p) {
	return glm::vec3(0.0,0.0,0.0);
}
