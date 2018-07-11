#include "Dialectric.hpp"

Dialectric::Dialectric(const glm::vec3& color, double idx_ref )
	: color(color)
	, idx_ref(idx_ref)
{}

Dialectric::~Dialectric()
{}

glm::vec3 Dialectric::get_color() {return color;}
double Dialectric::get_idx_ref() {return idx_ref;}
