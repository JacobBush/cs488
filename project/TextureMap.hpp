#pragma once
#include "ObjectMap.hpp"
#include <glm/glm.hpp>
#include <string>

class TextureMap : public ObjectMap {
public:
  virtual ~TextureMap();
  TextureMap(const char *filename);

  glm::vec3 get_color_at_point(double x, double y);

private:
  uint map_width;
  uint map_height;
  uint map_length;
  glm::vec3 *map_data;

  void readBMP(const char *filename);
};
