#pragma once
#include <glm/glm.hpp>
#include "Material.hpp"

class Dialectric : public Material {
public:
  Dialectric(const glm::vec3& color, double idx_ref);
  virtual ~Dialectric();

  glm::vec3 get_color();
  double get_idx_ref();

private:
  glm::vec3 color;
  double idx_ref;
};
