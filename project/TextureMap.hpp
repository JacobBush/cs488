#pragma once
#include "ObjectMap.hpp"
#include <glm/glm.hpp>
#include <vector>

class TextureMap : public ObjectMap {
public:
  virtual ~TextureMap();
  TextureMap(const char *filename);
  glm::vec3 get_color_at_point(double x, double y);
};
