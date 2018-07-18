#pragma once
#include <glm/glm.hpp>
#include <vector>

class ObjectMap {
public:
  virtual ~ObjectMap();

protected:
  ObjectMap(const char* filename);

  glm::vec4 get_value_at_point(uint x, uint y);

  uint width;
  uint height;
  std::vector<glm::vec4> map_data;

  void readPNG(const char *filename);
};

