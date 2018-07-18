#include "TextureMap.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include "math.h"

TextureMap::~TextureMap() {}
TextureMap::TextureMap(const char* filename) : ObjectMap(filename) {}

/*
 * x,y need to be in [0.0,1.0]
 * will return color  at the point x,y as though they were in [0,width] and [0, height]
 * color is [r,g,b,a]
 */
glm::vec3 TextureMap::get_color_at_point(double x, double y) {
	if (x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0) {
		std::cout << "Call to TextureMap::get_color_at_point(double x, double y) with parameters" << std::endl;
		std::cout << "x: " << x << ", y: " << y << "while they must be in [0.0, 1.0]" << std::endl;
		return glm::vec3(0.0,0.0,0.0);
	}
    glm::vec4 color_rgba = get_value_at_point(round(x * (width - 1)), round (y * (height - 1)));
    return glm::vec3(color_rgba); // truncate alpha
}
