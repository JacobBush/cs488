#pragma once
#include "Light.hpp"
#include <nanoflann.hpp>
#include <list>

class PhotonMap {
public:
	PhotonMap();
	PhotonMap(std::list<Light *> lights);
	~PhotonMap();

private:

std::list<Light *> lights;

};
