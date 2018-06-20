#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"

struct Triangle
{
	size_t v1;
	size_t v2;
	size_t v3;

	Triangle( size_t pv1, size_t pv2, size_t pv3 )
		: v1( pv1 )
		, v2( pv2 )
		, v3( pv3 )
	{}
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  ~Mesh();
  Mesh( const std::string& fname );
  double intersection(glm::vec3 a, glm::vec3 b) override;
  glm::vec3 get_normal_at_point(glm::vec3 p) override;
  
private:
    glm::vec3 triangle_norm(const Triangle & tri);
    bool point_on_triangle(glm::vec3 p, const Triangle & tri);
    double triangle_intersection(const Triangle & tri, glm::vec3 a, glm::vec3 b);

	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;

	glm::vec3 min, max; // for bounding box / sphere

	glm::mat4 *T_inv;

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
