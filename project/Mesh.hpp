#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"
#include "Triangle.hpp"

// A polygonal mesh.
class Mesh : public Primitive {
public:
  ~Mesh();
  Mesh( const std::string& fname );
  Intersection *intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) override;
  glm::vec3 get_normal_at_point(glm::vec3 p, Intersection *intersection) override;
  glm::vec2 map_to_2d(glm::vec3 p) override;
  // bounding volume
  glm::vec3 get_bb_bottom_left_corner() override;
  glm::vec3 get_bb_top_right_corner() override;

protected:
	const double BB_EPSILON = CUBE_BB_EPSILON * 256.0;
	const bool SPECIAL_BOUNDING_BOX_RENDERING = false;
  
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
