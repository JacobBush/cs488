#pragma once
#include "ObjectMap.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>

class TextureMap : public ObjectMap {
public:
  virtual ~TextureMap();
  TextureMap(const char *filename);

  glm::vec4 get_color_at_point(double x, double y);

private:
  uint width;
  uint height;
  // uint length;
  std::vector<glm::vec4> map_data;

  void readPNG(const char *filename);
};
