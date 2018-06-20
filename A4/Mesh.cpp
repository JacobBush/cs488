#include <iostream>
#include <fstream>
#include <math.h>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"


Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

glm::vec3 Mesh::triangle_norm(const Triangle & tri) {
	glm::vec3 a = m_vertices.at(tri.v1);
	glm::vec3 b = m_vertices.at(tri.v2);
	glm::vec3 c = m_vertices.at(tri.v3);
	return glm::normalize(glm::cross(b-a, c-a));
}

bool Mesh::point_on_triangle(glm::vec3 p, const Triangle & tri) {
	glm::vec3 P0 = m_vertices.at(tri.v1);
	glm::vec3 P1 = m_vertices.at(tri.v2);
	glm::vec3 P2 = m_vertices.at(tri.v3);

	glm::vec3 C0 = P1-P0;
	glm::vec3 C1 = P2-P0;
	glm::vec3 R = p-P0;

	// Solve the 2 equation system
	double D = glm::determinant(glm::mat2(glm::vec2(C0),glm::vec2(C1)));
	if (glm::abs(D) < EPSILON) return false;

	double D0 = glm::determinant(glm::mat2(glm::vec2(R),glm::vec2(C1)));
	double D1 = glm::determinant(glm::mat2(glm::vec2(C0),glm::vec2(R)));

	double beta = D0 / D;
	double gamma = D1 / D;

	if (beta >= -EPSILON && gamma >= -EPSILON && beta + gamma - 1.0 <= EPSILON) {
		// Check to see that we satisfy the third equation
		if (glm::abs(beta * C0.z + gamma * C1.z - R.z) < EPSILON) {
			return true;
		}
	}
	return false;
}

double Mesh::triangle_intersection(const Triangle & tri, glm::vec3 a, glm::vec3 b) {
	glm::vec3 P0 = m_vertices.at(tri.v1);
	glm::vec3 P1 = m_vertices.at(tri.v2);
	glm::vec3 P2 = m_vertices.at(tri.v3);
	
	glm::vec3 C0 = P1-P0;
	glm::vec3 C1 = P2-P0;
	glm::vec3 C2 = -(b-a);
	glm::vec3 R = a - P0;

	double D = glm::determinant(glm::mat3(C0,C1,C2));
	if (glm::abs(D) < EPSILON) return nan("");

	double D0 = glm::determinant(glm::mat3(R,C1,C2));
	double D1 = glm::determinant(glm::mat3(C0,R,C2));
	double D2 = glm::determinant(glm::mat3(C0,C1,R));

	double beta = D0 / D;
	double gamma = D1 / D;
	double t = D2 / D;

	if (beta >= -EPSILON && gamma >= -EPSILON && beta + gamma - 1.0 <= EPSILON) {
		return t;
	} else {
		return nan("");
	}
}

double Mesh::intersection(glm::vec3 a, glm::vec3 b) {
	// where (b-a) defines a ray.
	double t = nan("");
	for (Triangle tri : m_faces) {
		double tprime = triangle_intersection(tri, a, b);
		if (isnan(t) || (!isnan(tprime) && tprime < t)) t = tprime;
	}
	return t;
}

glm::vec3 Mesh::get_normal_at_point(glm::vec3 p) {
	for (Triangle tri : m_faces) {
		if (point_on_triangle(p, tri)) {
			return triangle_norm(tri);
		}
	}
	return glm::vec3(0.0);
}
