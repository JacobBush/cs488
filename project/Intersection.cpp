#include "Intersection.hpp"

Intersection::~Intersection() {
	if (tri != NULL) delete tri;
}

Intersection::Intersection(): has_intersected(false), t(nan("")), node(NULL), invtrans(glm::mat4()), local_intersection(glm::vec3()), tri(NULL) {}


Intersection::Intersection(double pt): Intersection() {
	has_intersected = std::isnan(pt) ? false : true;
	t = pt;
}

Intersection::Intersection(double pt, GeometryNode *pnode): Intersection(pt) {
	node = pnode;
}
