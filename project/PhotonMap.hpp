#pragma once
#include "SpacePartition.hpp"
#include "Light.hpp"
#include "nanoflann.hpp"
#include "utils.h" // PointCloud for nanoflann
#include <list>
#include <glm/glm.hpp>

// construct a kd-tree index:
typedef nanoflann::KDTreeSingleIndexAdaptor<
	nanoflann::L2_Simple_Adaptor<double, PointCloud<double> > ,
	PointCloud<double>,
	3 /* dim */
	> my_kd_tree_t;

class PhotonMap {
public:
	// PhotonMap();
	PhotonMap(const std::list<Light *> &lights, SpacePartition *space_partition);
	~PhotonMap();

	double get_light_density_at_point(const glm::vec3 &p);

private:
	static constexpr double PI = 3.14159265359;
	static const uint MAX_HITS = 20;
	static const uint NUM_PHOTONS_PER_LIGHT = 5000000;
	static const uint NUM_PHOTONS_TO_SEARCH = 10;

	glm::vec3 ray_point_at_parameter(const glm::vec3 & a, const glm::vec3 & b, double t);

	glm::vec3 get_random_point_on_sphere(); // unit sphere
	// is_reflected_or_refracted since only care about caustics
	void cast_photon(glm::vec3 a, glm::vec3 b, bool is_reflected_or_refracted, uint depth);
	void cast_photons_from_light(Light *light);
	void initialize();

	PointCloud<double> photon_cloud;
	const std::list<Light *> &lights;
	SpacePartition *space_partition;
	my_kd_tree_t *index;

	uint num_photons_captured;
};
