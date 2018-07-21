#include "Dialectric.hpp"

Dialectric::Dialectric(const glm::vec3& color, double idx_ref, double shininess )
	: color(color)
	, idx_ref(idx_ref)
	, shininess(shininess)
{}

Dialectric::~Dialectric()
{}

glm::vec3 Dialectric::get_color() {return color;}
double Dialectric::get_idx_ref() {return idx_ref;}
double Dialectric::get_shininess() {return shininess;}
