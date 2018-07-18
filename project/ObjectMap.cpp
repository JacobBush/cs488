#include "ObjectMap.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "math.h"
#include "lodepng/lodepng.h"

ObjectMap::ObjectMap(const char* filename)
{
	readPNG(filename);
}

ObjectMap::~ObjectMap()
{}

void ObjectMap::readPNG(const char* filename) {
    std::vector<unsigned char> image;
    unsigned error = lodepng::decode(image, width, height, filename);
    if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    for (uint i = 0; i < 4 * width * height; i += 4) {
        unsigned char r = image.at(i);
        unsigned char g = image.at(i + 1);
        unsigned char b = image.at(i + 2);
        unsigned char a = image.at(i + 3);
        map_data.push_back(glm::vec4((double)r/255.0,(double)g/255.0,(double)b/255.0,(double)a/255.0));
    }
}

glm::vec4 ObjectMap::get_value_at_point(uint x, uint y) {
    return map_data.at(x + y * width);
}
