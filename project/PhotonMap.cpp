#include "PhotonMap.hpp"
#include <cstdlib>
#include "Intersection.hpp"
#include "math.h"

PhotonMap::PhotonMap(const std::list<Light *> &lights, SpacePartition *space_partition):
	lights(lights), space_partition(space_partition), photon_cloud(PointCloud<double>()),
	index(NULL), num_photons_captured(0)
{
	initialize();
}

PhotonMap::~PhotonMap() {
	if (index != NULL) delete index;
}

glm::vec3 PhotonMap::ray_point_at_parameter(const glm::vec3 & a, const glm::vec3 & b, double t) {
	return a + (float)t*(b-a); // doesn't like double
}

glm::vec3 PhotonMap::get_random_point_on_sphere() {
	double z = 2.0 * (double)rand()/RAND_MAX - 1; // [-1,1]
	double t = 2.0 * PI * (double)rand()/RAND_MAX; // [0, 2*PI]
  	double x = glm::sqrt(1 - z*z) * glm::cos(t);
  	double y = glm::sqrt(1 - z*z) * glm::sin(t);
  	return glm::vec3(x, y, z);
}

void PhotonMap::cast_photon(glm::vec3 a, glm::vec3 b, bool is_reflected_or_refracted, uint depth) {
	if (depth > MAX_HITS) return;
	Intersection * intersect = space_partition->intersect(a,b);
	if (!intersect->has_intersected) return;
	glm::vec3 p_model = intersect->local_intersection;
  	glm::vec3 N_model = intersect->node->m_primitive->get_normal_at_point(p_model, intersect);
  	glm::vec3 N = glm::vec3(glm::transpose(intersect->invtrans) * glm::vec4(N_model, 1.0));
  	N = glm::normalize(N);
	glm::vec3 pt = ray_point_at_parameter(a, b, intersect->t);

	double p = intersect->node->get_percent_absoption(a,b,N, intersect);
	double q = intersect->node->get_percent_reflection(a,b,N, intersect);
	// double r = intersect->node->get_percent_refration();
	double k = (double)rand()/RAND_MAX; // random number [0,1]
	if (k < p) { // absorb
		if (!is_reflected_or_refracted) {
			delete intersect;
			return;
		}
		photon_cloud.pts.resize(photon_cloud.pts.size() + 1);
		photon_cloud.pts[photon_cloud.pts.size() - 1].x = pt.x;
		photon_cloud.pts[photon_cloud.pts.size() - 1].y = pt.y;
		photon_cloud.pts[photon_cloud.pts.size() - 1].z = pt.z;
		glm::vec3 ray = glm::normalize(b - a);
		photon_cloud.pts[photon_cloud.pts.size() - 1].phi = acos(ray.y); // spherical incidence
		photon_cloud.pts[photon_cloud.pts.size() - 1].theta = atan(ray.z/ray.x);
		num_photons_captured++;
		delete intersect;
	} else if (k < p + q) { // reflect
		glm::vec3 ref_endpt = intersect->node->get_reflect_endpoint(a,pt,N);
		delete intersect;
		cast_photon(pt, ref_endpt, true, depth + 1);
	} else { // refract
		glm::vec3 ref_endpt = intersect->node->get_refract_endpoint(a,pt,N);
		delete intersect;
		cast_photon(pt, ref_endpt, true, depth + 1);
	}
}

void PhotonMap::cast_photons_from_light(Light *light) {
	for (int i = 0; i < NUM_PHOTONS_PER_LIGHT; i++) {
		glm::vec3 a = light->position;
		glm::vec3 b = a + get_random_point_on_sphere();
		cast_photon(a,b,false, 0);
	}
}

// Will return a scalar for ambient light component
// based on max distance to find NUM_PHOTONS_TO_SEARCH photons around p
double PhotonMap::get_light_density_at_point(const glm::vec3 &p) {

	if (NUM_PHOTONS_TO_SEARCH > num_photons_captured) return 0.0;
	// do a knn search
	size_t ret_index = 0;
	double out_dist_sqr = 0.0;
	nanoflann::KNNResultSet<double> resultSet(NUM_PHOTONS_TO_SEARCH);
	resultSet.init(&ret_index, &out_dist_sqr );
	double query_pt[3] = {p.x, p.y, p.z};
	index->findNeighbors(resultSet, query_pt, nanoflann::SearchParams(10));

	// std::cout << "knnSearch(nn="<<NUM_PHOTONS_TO_SEARCH<<"): \n";
	// std::cout << "ret_index=" << ret_index << " out_dist_sqr=" << out_dist_sqr << std::endl;

	// look for "area" searched - decent approximation
	if (out_dist_sqr <= 0.0000001) return 0.0;
	double area_searched = PI * out_dist_sqr;

	// double fraction_light_area_searched = 0; // times the number of lights
	// for (Light * light : lights) {
	// 	double distance = glm::distance(light->position, p);
	// 	double total_sphere_area = 4.0 * PI * distance * distance;
	// 	if (total_sphere_area <= 0.001) continue;
	// 	fraction_light_area_searched += area_searched/total_sphere_area;
	// }
	// fraction_light_area_searched /= (double)lights.size(); // Normalize
	// double num_photons_expected = fraction_light_area_searched * num_photons_captured;
	// std::cout << num_photons_captured << std::endl;
	// std::cout << num_photons_expected << std::endl;
	// std::cout << num_photons_expected << std::endl;
	// std::cout << (NUM_PHOTONS_TO_SEARCH/(area_searched * NUM_PHOTONS_PER_LIGHT * lights.size())) << std::endl;
	return (NUM_PHOTONS_TO_SEARCH/(area_searched * num_photons_captured));
}

void PhotonMap::initialize() {
	for (Light * light : lights) {
		cast_photons_from_light(light);
	}
	std::cout << "Photons captured for caustics :" << num_photons_captured  << std::endl;
	index = new my_kd_tree_t(3 /*dim*/, photon_cloud, nanoflann::KDTreeSingleIndexAdaptorParams(2000 /* max leaf */) );
	index->buildIndex();
}