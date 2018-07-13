#include <iostream>
#include <fstream>
#include <math.h>
#include <limits>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"
#include "Primitive.hpp"


Mesh::~Mesh() {
	if (T_inv != NULL) delete T_inv;
}

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	glm::vec3 min, max;
	bool have_read_val = false;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {

			ifs >> vx >> vy >> vz;

			// For bounding volume
			if (have_read_val) {
				min = glm::vec3(
					glm::min((double)min.x, vx),
					glm::min((double)min.y, vy),
					glm::min((double)min.z, vz)
				);

				max = glm::vec3(
					glm::max((double)max.x, vx),
					glm::max((double)max.y, vy),
					glm::max((double)max.z, vz)
				);
			} else {
				min = glm::vec3(vx, vy, vz);
				max = glm::vec3(vx, vy, vz);
				have_read_val = true;
			}

			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}

	// The buffer helps to deal simply with surgaces like planes
	glm::mat4 S = glm::scale(max-min + glm::vec3(2.0 * BB_EPSILON));
	glm::mat4 T = glm::translate(min - glm::vec3(BB_EPSILON));

	T_inv = new glm::mat4(glm::inverse(T * S));
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
	return glm::cross(b-a, c-a);
}

bool Mesh::point_on_triangle(glm::vec3 p, const Triangle & tri) {
	glm::vec3 P0 = m_vertices.at(tri.v1);
	glm::vec3 P1 = m_vertices.at(tri.v2);
	glm::vec3 P2 = m_vertices.at(tri.v3);
	
	glm::vec3 C0 = P1-P0;
	glm::vec3 C1 = P2-P0;
	glm::vec3 C2 = glm::vec3(0.0,0.0,1.0);
	glm::vec3 R = p - P0;

	double D = glm::determinant(glm::mat3(C0,C1,C2));
	if (glm::abs(D) < MESH_EPSILON) return nan("");

	double D0 = glm::determinant(glm::mat3(R,C1,C2));
	double D1 = glm::determinant(glm::mat3(C0,R,C2));

	double beta = D0 / D;
	double gamma = D1 / D;

	if (beta >= -MESH_EPSILON && gamma >= -MESH_EPSILON && beta + gamma - 1.0 <= MESH_EPSILON) {
		return true;
	} else {
		return false;
	}
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
	if (glm::abs(D) < MESH_EPSILON) return nan("");

	double D0 = glm::determinant(glm::mat3(R,C1,C2));
	double D1 = glm::determinant(glm::mat3(C0,R,C2));
	double D2 = glm::determinant(glm::mat3(C0,C1,R));

	double beta = D0 / D;
	double gamma = D1 / D;
	double t = D2 / D;

	if (beta >= -MESH_EPSILON && gamma >= -MESH_EPSILON && beta + gamma - 1.0 <= MESH_EPSILON
		&& t >= MESH_EPSILON) { // t is some non-zero distance along ray to b
		return t;
	} else {
		return nan("");
	}
}

Intersection *Mesh::intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) {
	// where (b-a) defines a ray.
	
	glm::vec3 aprime = glm::vec3(*T_inv * glm::vec4(a, 1));
	glm::vec3 bprime = glm::vec3(*T_inv * glm::vec4(b, 1));

	if (SPECIAL_BOUNDING_BOX_RENDERING) {
		return Cube(true).intersection(aprime, bprime, prev_intersection);
	}

	// only bounding box if more polygons than the box (if it were made of triangles)
	if (m_faces.size() > 12 && 
		!Cube(true).intersection(aprime, bprime, prev_intersection)->has_intersected) {
		return new Intersection(); // We don't hit bounding box
	}

	Intersection *i = new Intersection();
	for (Triangle tri : m_faces) {
		double tprime = triangle_intersection(tri, a, b);
		if (!i->has_intersected || i->t < MESH_EPSILON || 
			!isnan(tprime) && tprime < i->t && tprime >= MESH_EPSILON) {
			// No need to use this really
			//if (prev_intersection && prev_intersection->tri == &tri) {
				// triangles shouldn't have self intersection
				//continue;
			//}
			delete i;
			i = new Intersection(tprime);
			i->tri = new Triangle(tri.v1, tri.v2, tri.v3);
		}
	}
	return i;
}

glm::vec3 Mesh::get_normal_at_point(glm::vec3 p, Intersection *intersection) {
	if (SPECIAL_BOUNDING_BOX_RENDERING) {
		glm::vec3 N_local = Cube(true).get_normal_at_point(glm::vec3(*T_inv * glm::vec4(p, 1)), intersection);
		// We don't need to transform back, since cube always aligned with axes
		return N_local;
		//return glm::vec3(glm::transpose(*T) * glm::vec4(N_local,1.0));
	}

	if (intersection != NULL && intersection->tri != NULL) {
		return triangle_norm(*(intersection->tri));
	}

	for (Triangle tri : m_faces) {
		if (point_on_triangle(p, tri)) {
			return triangle_norm(tri);
		}
	}
	return glm::vec3(0.0);
}

glm::vec2 Mesh::map_to_2d(glm::vec3 p) {
	return glm::vec2(0.0,0.0);
}
