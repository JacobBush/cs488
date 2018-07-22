#include "PhotonMap.hpp"

PhotonMap::PhotonMap():
	lights(std::list<Light *>())
{
}

PhotonMap::PhotonMap(std::list<Light *> lights):
	lights(lights)
{
}

PhotonMap::~PhotonMap() {
}