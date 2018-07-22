#include "Primitive.hpp"
#include <iostream>
#include <math.h>
#include <glm/ext.hpp>

#include "polyroots.hpp"

const double PI = 3.14159265359;
double sq(double x) {return x*x;}

glm::vec3 Primitive::ray_point_at_parameter(const glm::vec3 &a, const glm::vec3 &b, double t) {
	return a + t*(b-a);
}

Intersection *solve_quadratic_intersect(double A, double B, double C, double E) {
	// Solves Ax^2 + Bx + B = 0 with E as epsilon
	double discriminant = sq(B) - 4.0*A*C;
	if (discriminant < -E) {
		// negative - no roots
		return new Intersection();
	} else if (glm::abs(discriminant) <= E) {
		// approximately 0 - 1 root
		double t = -2.0*C/B;
		if (t >= E) return new Intersection(t);
		else return new Intersection();
	} else {
		// 2 roots
		double sqrt_disc = glm::sqrt(discriminant);
		double t1 = -2.0*C/(B + sqrt_disc);
		double t2 = -2.0*C/(B - sqrt_disc);
		if (t1 < E && t2 < E) return new Intersection();
		else if (t1 < E) return new Intersection(t2);
		else if (t2 < E) return new Intersection(t1);
		else return new Intersection(glm::min(t1,t2));
	}
}

double Primitive::plane_intersection(glm::vec3 p0, glm::vec3 N, glm::vec3 a, glm::vec3 b) {
	double den = glm::dot(N, (b-a));
	if (glm::abs(den) >= PLANE_EPSILON) {
		return glm::dot(N, (p0-a)) / den;
	} else {
		return nan("");
	}
}

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

Torus::~Torus()
{
}

Cylinder::~Cylinder()
{
}

Intersection *Primitive::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// where (b-a) defines a ray.
	// -1 -> no intersection
	return new Intersection();
}

Intersection *Sphere::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// where (b-a) defines a ray.
	// -1 -> no intersection

	// m_pos = [0,0,0]
	// m_radius = 1
	double A = glm::dot(b-a, b-a);
	double B = 2.0 * glm::dot(b-a, a);
	double C = glm::dot(a, a) - 1;
	return solve_quadratic_intersect(A,B,C,SPHERE_EPSILON);
}

Intersection *Cube::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// where (b-a) defines a ray.
	double E = is_bounding_box ? CUBE_BB_EPSILON : CUBE_EPSILON;
	double t = nan("");
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			double tprime = intersect_side(i, j, a, b);
			if (std::isnan(t) || t < E || (!std::isnan(tprime) && tprime >= E && tprime < t))
				t = tprime;
		}
	}
	if (std::isnan(t) || t < E) {
		return new Intersection();
	}
	return new Intersection(t);
}

Intersection *NonhierBox::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// where (b-a) defines a ray.
	double t = nan("");
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			double tprime = intersect_side(i, j, a, b);
			if (std::isnan(t) || t < CUBE_EPSILON || (!std::isnan(tprime) && tprime >= CUBE_EPSILON && tprime < t))
				t = tprime;
		}
	}
	if (std::isnan(t) || t < CUBE_EPSILON) {
		return new Intersection();
	}
	return new Intersection(t);
}

Intersection *NonhierSphere::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// where (b-a) defines a ray.
	// -1 -> no intersection
	//std::cout << "in NonhierSphere::intersection" << std::endl;

	double A = glm::dot(b-a, b-a);
	double B = 2.0 * glm::dot(b-a, a-m_pos);
	double C = glm::dot(a-m_pos, a-m_pos) - m_radius*m_radius;
	return solve_quadratic_intersect(A,B,C,SPHERE_EPSILON);
}

bool Cube::point_on_side(glm::vec3 p, uint side, bool front) {
	double E = is_bounding_box ? CUBE_BB_EPSILON : CUBE_EPSILON;
	double sign = front ? 1.0 : -1.0;

	if (glm::abs(p[side] - 0.5 - sign*0.5) > E) return false;
	side = (side + 1) % 3;
	if (glm::abs(p[side] - 0.5) > 0.5 + E) return false;
	side = (side + 1) % 3;
	if (glm::abs(p[side] - 0.5) > 0.5 + E) return false;
	return true;
}

bool NonhierBox::point_on_side(glm::vec3 p, uint side, bool front) {
	double sign = front ? 1.0 : -1.0;
	glm::vec3 center = m_pos + glm::vec3(m_size/2.0);

	if (glm::abs(p[side] - center[side] - sign*m_size/2.0) > CUBE_EPSILON) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - center[side]) > m_size/2.0 + CUBE_EPSILON) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - center[side]) > m_size/2.0 + CUBE_EPSILON) {return false;}
	return true;
}

double Cube::intersect_side(uint side, bool front, glm::vec3 a, glm::vec3 b) {
	double sign = front ? 1.0 : -1.0;

	glm::vec3 p0 = glm::vec3(0.5);
	glm::vec3 N = glm::vec3(0.0,0.0,0.0);
	p0[side] += sign*0.5;
	N[side] = sign;

	double t = plane_intersection(p0, N, a, b);
	if (std::isnan(t)) return nan("");
	glm::vec3 p = ray_point_at_parameter(a,b,t);
	if (!point_on_side(p, side, front)) {
		return nan("");
	}

	return t;
}

double NonhierBox::intersect_side(uint side, bool front, glm::vec3 a, glm::vec3 b) {
	double sign = front ? 1.0 : -1.0;

	glm::vec3 p0 = m_pos + glm::vec3(m_size/2.0);
	glm::vec3 N = glm::vec3(0.0,0.0,0.0);
	p0[side] += sign*m_size/2.0;
	N[side] = sign;

	double t = plane_intersection(p0, N, a, b);
	if (std::isnan(t)) return nan("");
	glm::vec3 p = ray_point_at_parameter(a,b,t);
	if (!point_on_side(p, side, front)) {
		return nan("");
	}

	return t;
}

glm::vec3 Primitive::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	return glm::vec3(0.0,0.0,0.0);
}

glm::vec3 Sphere::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	return p;
}

glm::vec3 Cube::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			if (point_on_side(p, i, j)) {
				glm::vec3 N = glm::vec3(0.0,0.0,0.0);
				N[i] = j ? 1.0 : -1.0;
				return N;
			}
		}
	}
}

glm::vec3 NonhierSphere::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	return p - m_pos;
}

glm::vec3 NonhierBox::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			if (point_on_side(p, i, j)) {
				glm::vec3 N = glm::vec3(0.0,0.0,0.0);
				N[i] = j ? 1.0 : -1.0;
				return N;
			}
		}
	}
}

//
// Torus
//

Intersection *Torus::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// by: cosinekitty.com/raytrace/chapter13_torus.html

	const glm::vec3 D = a;
	const glm::vec3 E = b-a;

	const double G = 4*(sq(E.x) + sq(E.z));
	const double H = 8*(D.x*E.x + D.z*E.z);
	const double I = 4*(sq(D.x) + sq(D.z));
	const double J = sq(E.x) + sq(E.y) + sq(E.z);
	const double K = 2*glm::dot(D, E);
	const double L = sq(D.x) + sq(D.y) + sq(D.z) + 1.0 - sq(r);

	double roots[4] = {0};

	const size_t nroots = quarticRoots(2*K/J, (2*J*L + sq(K) - G)/sq(J), (2*K*L - H)/sq(J), (sq(L) - I)/sq(J), roots);

	// No intersection of ray and torus
	if (!nroots) return new Intersection();

	double root = roots[0];
	for (int i = 1; i < nroots; i++) {
		if (root < TORUS_EPSILON || (roots[i] < root && roots[i] >= TORUS_EPSILON))
			root = roots[i];
	}

	// All of the roots were bad
	if (root < TORUS_EPSILON) return new Intersection();

	return new Intersection(root);
}


glm::vec3 Torus::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	// by: cosinekitty.com/raytrace/chapter13_torus.html
	// Assuming the point is on the surface
	double alpha = 1 - 1.0/glm::sqrt(sq(p.x) + sq(p.z));
	return glm::normalize(glm::vec3(alpha*p.x, p.y, alpha*p.z));
}

//
// Cylinder
//

Intersection *Cylinder::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// From https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html
	glm::vec3 D = b-a;
	double A = sq(D.x) + sq(D.z);
	double B = 2*(a.x*D.x + a.z*D.z);
	double C = sq(a.x) + sq(a.z) - 1;
	Intersection *i = solve_quadratic_intersect(A,B,C,CYLINDER_EPSILON);
	glm::vec3 p = ray_point_at_parameter(a,b,i->t);
	if (p.y <= 0.5 && p.y >= -0.5) { // cylinder of height 1
		return i;
	}
	delete i;
	// check intersect with top plane or bottom plane
	double sign = p.y > 0.5 ? 1.0 : -1.0;
	double t = plane_intersection(glm::vec3(0.0,sign * 0.5,0.0), glm::vec3(0.0,sign,0.0), a, b);
	if (t <= SPHERE_EPSILON) return new Intersection();
	p = ray_point_at_parameter(a,b,t);
	if (sq(p.x) + sq(p.z) <= 1.0) return new Intersection(t);
	else return new Intersection();
}


glm::vec3 Cylinder::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	// Assuming that the point is on the surface
	if (p.y <= 0.5 - SPHERE_EPSILON && p.y >= -0.5 + SPHERE_EPSILON)
		return glm::vec3(p.x, 0.0, p.z);
	else if (p.y > 0.5 - SPHERE_EPSILON) return glm::vec3(0.0,1.0,0.0);
	else return glm::vec3(0.0,-1.0,0.0);
}

// texture & bump mapping

double bound_by_range(double x, double a, double b) {
	if (x < a) x = a;
	if (x > b) x = b;
	return x;
}

glm::vec2 clamp_vec2(glm::vec2 v, double a, double b) {
	return glm::vec2(bound_by_range(v.x, a, b), bound_by_range(v.y, a, b));
}


glm::vec2 Primitive::map_to_2d(glm::vec3 p) {
	return glm::vec2(0.0,0.0);
}

glm::vec2 NonhierSphere::map_to_2d(glm::vec3 p) { // no checks for if on sphere
	return Sphere().map_to_2d(glm::normalize(p - m_pos));
}

glm::vec2 NonhierBox::map_to_2d(glm::vec3 p) {
	return Cube().map_to_2d((p - m_pos) / m_size);
}

glm::vec2 Sphere::map_to_2d(glm::vec3 p) { // no checks for if on sphere
	double theta = atan2(p.z,p.x) + PI; // range is [-PI, PI] originally
    double phi = acos(bound_by_range(p.y, -1.0, 1.0));
    return clamp_vec2(glm::vec2(theta/(2.0 * PI), phi/PI), 0.0, 1.0);
}

glm::vec2 Cube::map_to_2d(glm::vec3 p) {
	// return Sphere().map_to_2d(glm::normalize(p - glm::vec3(0.5, 0.5, 0.5))); // bounding sphere
	/* 0 r 0 0
	 * t f b k
	 * 0 l 0 0
	 * 0 0 0 0
	 * 
	 * Assuming (0,0) bot left corner, (1,1) top right corner
	 */

	glm::vec2 v = glm::vec2(0.0);

	if (glm::abs(p.x - 1.0) < CUBE_EPSILON) { // right 
		v = glm::vec2(p.y*0.25 + 0.25, p.z * 0.25 + 0.5);
	} else if (glm::abs(p.x) < CUBE_EPSILON) { // left
		v = glm::vec2(p.y*0.25 + 0.25, p.z * 0.25);
	} else if (glm::abs(p.y - 1.0) < CUBE_EPSILON) { // top
		v = glm::vec2(p.z*0.25, glm::abs(1.0 - p.x) * 0.25 + 0.25);
	} else if (glm::abs(p.y) < CUBE_EPSILON) { // bottom
		v = glm::vec2(glm::abs(1.0 - p.z)*0.25 + 0.5, glm::abs(1.0 - p.x) * 0.25 + 0.25);
	} else if (glm::abs(p.z - 1.0) < CUBE_EPSILON) { // back
		v = glm::vec2(p.y*0.25 + 0.75, p.x * 0.25 + 0.25);
	} else if (glm::abs(p.z) < CUBE_EPSILON) { // front
		v = glm::vec2(glm::abs(1.0 - p.y)*0.25 + 0.25, p.x * 0.25 + 0.25);
	}
	return clamp_vec2(v, 0.0, 1.0);
}

glm::vec2 Torus::map_to_2d(glm::vec3 p) {
	double theta = asin(bound_by_range(p.y/r, -1.0, 1.0));
	double phi = acos(bound_by_range(p.x/(1 + r * cos(theta)), -1.0, 1.0)); // can calculate with z as well
	theta += PI/2.0; // range [-pi/2, pi/2]
	return clamp_vec2(glm::vec2(theta/PI, phi/PI), 0.0, 1.0);
}

glm::vec2 Cylinder::map_to_2d(glm::vec3 p) {
	double theta = atan2(p.z,p.x) + PI; // range is [-PI, PI] originally
	double x = bound_by_range(theta/(2.0 * PI), 0.0, 1.0);
	return glm::vec2(x, bound_by_range(p.y + 0.5, 0.0, 1.0));
}

glm::vec3 Primitive::get_bb_bottom_left_corner() {
	return glm::vec3(0.0);
}

glm::vec3 Primitive::get_bb_top_right_corner() {
	return glm::vec3(0.0);
}

glm::vec3 Sphere::get_bb_bottom_left_corner() {
	return glm::vec3(-1.0);
}

glm::vec3 Sphere::get_bb_top_right_corner() {
	return glm::vec3(1.0);
}

glm::vec3 NonhierSphere::get_bb_bottom_left_corner() {
	return m_pos - glm::vec3(m_radius);
}

glm::vec3 NonhierSphere::get_bb_top_right_corner() {
	return m_pos + glm::vec3(m_radius);
}

glm::vec3 Cube::get_bb_bottom_left_corner() {
	return glm::vec3(0.0);
}

glm::vec3 Cube::get_bb_top_right_corner() {
	return glm::vec3(1.0);
}

glm::vec3 NonhierBox::get_bb_bottom_left_corner() {
	return m_pos;
}

glm::vec3 NonhierBox::get_bb_top_right_corner() {
	return m_pos + glm::vec3(m_size);
}

glm::vec3 Cylinder::get_bb_bottom_left_corner() {
	return -glm::vec3(1.0, 0.5, 1.0);
}

glm::vec3 Cylinder::get_bb_top_right_corner() {
	return glm::vec3(1.0, 0.5, 1.0);
}

glm::vec3 Torus::get_bb_bottom_left_corner() {
	return -glm::vec3(1 + r, r, 1 + r);
}

glm::vec3 Torus::get_bb_top_right_corner() {
	return glm::vec3(1 + r, r, 1 + r);
}
