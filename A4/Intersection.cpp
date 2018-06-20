#include "Intersection.hpp"

Intersection::~Intersection() {
	if (tri != NULL) delete tri;	
}
