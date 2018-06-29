#include "Primitive.hpp"
#include <iostream>
#include <math.h>
#include <glm/ext.hpp>

#include "polyroots.hpp"

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

glm::vec3 Primitive::ray_point_at_parameter(const glm::vec3 &a, const glm::vec3 &b, double t) {
	return a + t*(b-a);
}

double Primitive::plane_intersection(glm::vec3 p0, glm::vec3 N, glm::vec3 a, glm::vec3 b) {
	double den = glm::dot(N, (b-a));
	if (glm::abs(den) >= PLANE_EPSILON) {
		return glm::dot(N, (p0-a)) / den;
	} else {
		return nan("");
	}
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

	double discriminant = B*B - 4.0*A*C;

	Intersection * i;

	if (discriminant < -SPHERE_EPSILON) {
		// negative - no roots
		i = new Intersection();
	} else if (glm::abs(discriminant) <= SPHERE_EPSILON) {
		// approximately 0 - 1 root
		i = new Intersection(-2.0*C/B);
	} else {
		// 2 roots
		double sqrt_disc = glm::sqrt(discriminant);
		double t1 = -2.0*C/(B + sqrt_disc);
		double t2 = -2.0*C/(B - sqrt_disc);

		i = new Intersection(glm::min(t1,t2));
	}
	if (!i->has_intersected || i->t < SPHERE_EPSILON) {
		delete i;
		i = new Intersection();
	}
	return i;
}

Intersection *Cube::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// where (b-a) defines a ray.
	double E = is_bounding_box ? CUBE_BB_EPSILON : CUBE_EPSILON;
	double t = nan("");
	for (uint i = 0; i < 3; i++) {
		for (uint j = 0; j <= 1; j++) {
			double tprime = intersect_side(i, j, a, b);
			if (isnan(t) || (!isnan(tprime) && tprime < t)) t = tprime;
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

	double discriminant = B*B - 4.0*A*C;

	Intersection *i;

	if (discriminant < -SPHERE_EPSILON) {
		// negative - no roots
		i = new Intersection();
	} else if (glm::abs(discriminant) <= SPHERE_EPSILON) {
		// approximately 0 - 1 root
		i = new Intersection(-2.0*C/B);
	} else {
		// 2 roots
		double sqrt_disc = glm::sqrt(discriminant);
		double t1 = -2.0*C/(B + sqrt_disc);
		double t2 = -2.0*C/(B - sqrt_disc);

		i = new Intersection(glm::min(t1,t2));
	}
	if (!i->has_intersected || i->t < SPHERE_EPSILON) {
		delete i;
		i = new Intersection();
	}
	return i;
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

double sq(double x) {return x*x;}

Intersection *Torus::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// by: cosinekitty.com/raytrace/chapter13_torus.html

	const glm::vec3 D = a;
	const glm::vec3 E = b-a;

	const double G = 4*sq(A)*(sq(E.x) + sq(E.z));
	const double H = 8*sq(A)*(D.x*E.x + D.z*E.z);
	const double I = 4*sq(A)*(sq(D.x) + sq(D.z));
	const double J = sq(E.x) + sq(E.y) + sq(E.z);
	const double K = 2*glm::dot(D, E);
	const double L = sq(D.x) + sq(D.y) + sq(D.z) + sq(A) - sq(B);

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
	double alpha = 1 - A/glm::sqrt(sq(p.x) + sq(p.z));
	return glm::normalize(glm::vec3(alpha*p.x, p.y, alpha*p.z));
}

//
// Cylinder
//

Intersection *Cylinder::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	return new Intersection();
}


glm::vec3 Cylinder::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	return glm::vec3();
}
