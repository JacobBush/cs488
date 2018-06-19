#pragma once

#include <glm/glm.hpp>

class Primitive {
public:
  virtual ~Primitive();
  virtual double intersection(glm::vec3 a, glm::vec3 b);
  virtual glm::vec3 get_normal_at_point(glm::vec3 p);

protected:
  // 10^-10
  const double EPSILON = 0.0001;
  double plane_intersection(glm::vec3 p0, glm::vec3 N, glm::vec3 a, glm::vec3 b);
  glm::vec3 ray_point_at_parameter(const glm::vec3 &a, const glm::vec3 &b, double t);
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
  double intersection(glm::vec3 a, glm::vec3 b) override;
  glm::vec3 get_normal_at_point(glm::vec3 p) override;
};

class Cube : public Primitive {
public:
  virtual ~Cube();
  double intersection(glm::vec3 a, glm::vec3 b) override;
  glm::vec3 get_normal_at_point(glm::vec3 p) override;
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  double intersection(glm::vec3 a, glm::vec3 b) override;
  glm::vec3 get_normal_at_point(glm::vec3 p) override;

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
  double intersection(glm::vec3 a, glm::vec3 b) override;
  glm::vec3 get_normal_at_point(glm::vec3 p) override;

private:
  double intersect_side(uint side, bool front, glm::vec3 a, glm::vec3 b);
  bool point_on_side(glm::vec3 p, uint side, bool front);

  glm::vec3 m_pos;
  double m_size;
};
