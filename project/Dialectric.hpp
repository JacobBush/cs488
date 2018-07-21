#pragma once
#include <glm/glm.hpp>
#include "Material.hpp"

class Dialectric : public Material {
public:
  Dialectric(const glm::vec3& color, double idx_ref, double shininess);
  virtual ~Dialectric();

  glm::vec3 get_color();
  double get_idx_ref();
  double get_shininess();

private:
  glm::vec3 color;
  double idx_ref;
  double shininess;
};
