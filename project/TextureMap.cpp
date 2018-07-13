#include "TextureMap.hpp"
#include <cstdio>
#include <glm/glm.hpp>
#include <iostream>
#include "math.h"

TextureMap::~TextureMap() {
	if (map_data) delete[] map_data;
}

TextureMap::TextureMap(const char* filename) {
	readBMP(filename);
}

/*
 * x,y need to be in [0.0,1.0]
 * will return color  at the point x,y as though they were in [0,width] and [0, height]
 */
glm::vec3 TextureMap::get_color_at_point(double x, double y) {
	if (x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0) {
		std::cout << "Call to TextureMap::get_color_at_point(double x, double y) with parameters" << std::endl;
		std::cout << "x: " << x << ", y: " << y << "while they must be in [0.0, 1.0]" << std::endl;
		return glm::vec3(0.0,0.0,0.0);
	}

	x *= (map_width - 1);
	y *= (map_height - 1);

	// We will round to integers
	// this should never be outside the range [0, width - 1] or [0, height - 1]
	uint a = round(x);
	uint b = round(y);

	uint idx = a + b * map_width;
	return map_data[idx];
}

/*
 * Reads bmp and stores in TextureMap memory
 * Taken from:
 * https://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file
 */
void TextureMap::readBMP(const char* filename) {
    int i;
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    map_width = *(int*)&info[18];
    map_height = *(int*)&info[22];

    uint size = 3 * map_width * map_height;
    unsigned char *data = new unsigned char[size]; // allocate 3 bytes per pixel
    fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
    fclose(f);

    map_data = new glm::vec3[map_width * map_height];

    for(i = 0; i < size; i += 3)
    {
    	map_data[i/3] = glm::vec3(
    		(double)data[i + 2] / 255.0,
    		(double)data[i + 1] / 255.0,
    		(double)data[i] / 255.0
    	);
    }
    delete[] data;
}
