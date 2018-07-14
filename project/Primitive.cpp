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
			if (isnan(t) || t < E || (!isnan(tprime) && tprime >= E && tprime < t))
				t = tprime;
		}
	}
	if (isnan(t) || t < E) {
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

	if (glm::abs(p[side] - 0.5 - sign*0.5) > E) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - 0.5) > 0.5 + E) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - 0.5) > 0.5 + E) {return false;}
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

	if (glm::abs(p[side] - center[side] - sign*m_size/2.0) > CUBE_EPSILON) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - center[side]) > m_size/2.0 + CUBE_EPSILON) {return false;}
	side = (side + 1) % 3;
	if (glm::abs(p[side] - center[side]) > m_size/2.0 + CUBE_EPSILON) {return false;}
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


Intersection *NonhierBox::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// where (b-a) defines a ray.
	double t = nan("");
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			double tprime = intersect_side(i, j, a, b);
			if (isnan(t) || (!isnan(tprime) && tprime < t)) t = tprime;
		}
	}
	if (isnan(t) || t < CUBE_EPSILON) {
		return new Intersection();
	}
	return new Intersection(t);
}

glm::vec3 Primitive::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	return glm::vec3(0.0,0.0,0.0);
}

glm::vec3 Sphere::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	return p;
}

glm::vec3 Cube::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	// We're going to assume that the point is on the cube
	p = glm::vec3(glm::abs(p.x), glm::abs(p.y), glm::abs(p.z));
	if (p.x < CUBE_EPSILON) return glm::vec3(-1.0, 0.0, 0.0);
	if (p.x > 1 - CUBE_EPSILON) return glm::vec3(1.0, 0.0, 0.0);
	if (p.y < CUBE_EPSILON) return glm::vec3(0.0, -1.0, 0.0);
	if (p.y > 1 - CUBE_EPSILON) return glm::vec3(0.0, 1.0, 0.0);
	if (p.y < CUBE_EPSILON) return glm::vec3(0.0, 0.0, -1.0);
	if (p.y > 1 - CUBE_EPSILON) return glm::vec3(0.0, 0.0, 1.0);
	// The point is inside the cube
	return glm::vec3();
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

glm::vec2 Primitive::map_to_2d(glm::vec3 p) {
	return glm::vec2(0.0,0.0);
}

glm::vec2 NonhierSphere::map_to_2d(glm::vec3 p) {
	return glm::vec2(0.0,0.0);
}

glm::vec2 NonhierBox::map_to_2d(glm::vec3 p) {
	return glm::vec2(0.0,0.0);
}

glm::vec2 Sphere::map_to_2d(glm::vec3 p) {
	// Using spherical coordinates: 
	//	http://mathworld.wolfram.com/SphericalCoordinates.html
	//	https://en.wikipedia.org/wiki/Spherical_coordinate_system

	// normalize p so that it is on the surface of the sphere (sphere is unit at origin)
	p = glm::normalize(p);

	double a = p.y; // radius 1
	double b = p.z/p.x;

	// account for numerical imprecision - required for domain of acos
	if (a < -1.0) a = -1.0;
	if (a > 1.0) a = 1.0;

	double theta = acos(a);
	double phi = atan(b);

	double x = theta / PI;
	double y = (phi + PI/2) / PI;

	if (x < 0.0) x = 0.0;
	if (x > 1.0) x = 1.0;
	if (y < 0.0) y = 0.0;
	if (y > 1.0) y = 1.0;

	return glm::vec2(x,y);
}

glm::vec2 Cube::map_to_2d(glm::vec3 p) {
	return glm::vec2(0.0,0.0);
}

glm::vec2 Torus::map_to_2d(glm::vec3 p) {
	return glm::vec2(0.0,0.0);
}

glm::vec2 Cylinder::map_to_2d(glm::vec3 p) {
	return glm::vec2(0.0,0.0);
}
