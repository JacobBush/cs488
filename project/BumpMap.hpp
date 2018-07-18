#pragma once
#include "ObjectMap.hpp"
#include <glm/glm.hpp>
#include <vector>

class BumpMap : public ObjectMap {
public:
  virtual ~BumpMap();
  BumpMap(const char *filename);

  glm::vec3 get_perturbed_normal_at_point(double x, double y);
};
