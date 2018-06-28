#pragma once

#include <glm/glm.hpp>
#include "Intersection.hpp"

class Primitive {
public:
  virtual ~Primitive();
  virtual Intersection *intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection);
  virtual glm::vec3 get_normal_at_point(glm::vec3 p, Intersection *intersection);

protected:
  // 10^-10
  const double EPSILON = 1.0/1024.0;
  const double SPHERE_EPSILON = EPSILON / 64.0;
  const double CUBE_BB_EPSILON = EPSILON / 8.0;
  const double CUBE_EPSILON = EPSILON / 64.0;
  const double PLANE_EPSILON = EPSILON / 64.0;
  const double MESH_EPSILON = EPSILON / 64.0;


  double plane_intersection(glm::vec3 p0, glm::vec3 N, glm::vec3 a, glm::vec3 b);
  glm::vec3 ray_point_at_parameter(const glm::vec3 &a, const glm::vec3 &b, double t);
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
  Intersection *intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) override;
  glm::vec3 get_normal_at_point(glm::vec3 p, Intersection *intersection) override;
};

class Cube : public Primitive {
public:
  Cube() {}
  Cube(bool isbb): is_bounding_box(isbb) {}

  virtual ~Cube();
  Intersection *intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) override;
  glm::vec3 get_normal_at_point(glm::vec3 p, Intersection *intersection) override;

  bool is_bounding_box;

private:
  double intersect_side(uint side, bool front, glm::vec3 a, glm::vec3 b);
  bool point_on_side(glm::vec3 p, uint side, bool front);
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  Intersection *intersection(glm::vec3 a, glm::vec3 b,Intersection * prev_intersection) override;
  glm::vec3 get_normal_at_point(glm::vec3 p, Intersection *intersection) override;

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  
  virtual ~NonhierBox();
  Intersection *intersection(glm::vec3 a, glm::vec3 b, Intersection * prev_intersection) override;
  glm::vec3 get_normal_at_point(glm::vec3 p, Intersection *intersection) override;

private:
  double intersect_side(uint side, bool front, glm::vec3 a, glm::vec3 b);
  bool point_on_side(glm::vec3 p, uint side, bool front);

  glm::vec3 m_pos;
  double m_size;
};

class Torus : public Primitive {

}

class Cylinder : public Primitive {
  
}
