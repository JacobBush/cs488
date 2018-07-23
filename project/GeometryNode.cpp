#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include "Dialectric.hpp"

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
	, m_texture_map( 0 )
	, m_bump_map ( 0 )
{
	m_nodeType = NodeType::GeometryNode;
}

void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the 
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and 
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}


void GeometryNode::setTextureMap( TextureMap * tm )
{
	m_texture_map = tm;
}

void GeometryNode::setBumpMap( BumpMap * bm )
{
	m_bump_map = bm;
}

glm::vec3 GeometryNode::get_bb_bottom_left_corner() {
	if (m_primitive == NULL) return glm::vec3(0.0);
	return m_primitive->get_bb_bottom_left_corner();
}

glm::vec3 GeometryNode::get_bb_top_right_corner() {
	if (m_primitive == NULL) return glm::vec3(0.0);
	return m_primitive->get_bb_top_right_corner();
}

double max_vec3(const glm::vec3 &v) {
	return glm::max(v.x, glm::max(v.y, v.z));
}

double GeometryNode::get_percent_absoption(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &N, Intersection *i) {
	TextureMap *texmap = i->node->m_texture_map;
	if (dynamic_cast<PhongMaterial*>(m_material) != nullptr) {
		PhongMaterial * pm = (PhongMaterial *)m_material;
		double max_spec = max_vec3(pm->get_ks());
		double max_diff = max_vec3(pm->get_kd());
		if (texmap != NULL) {
			glm::vec2 tex_coord = i->node->m_primitive->map_to_2d(i->local_intersection);
			max_diff = max_vec3(texmap->get_color_at_point(tex_coord.x, tex_coord.y));
		}
		if (max_spec == max_diff) return 0.5;
		return max_diff/(max_diff + max_spec);
  	} else if (dynamic_cast<Dialectric *>(m_material) != nullptr) {
		Dialectric * dm = (Dialectric *)m_material;
		return max_vec3(dm->get_color()); // large color == more dialectric
	} else if (texmap != NULL) {
		return 1.0;
	} else {
    	std::cout << "Unkown material type" << std::endl;
    	throw;
	}
	return 1.0;
}

double GeometryNode::get_percent_reflection(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &N_orig, Intersection *i) {
	if (dynamic_cast<PhongMaterial*>(m_material) != nullptr) {
		return 1.0 - get_percent_absoption(a,b,N_orig, i); // no transmittance
  	} else if (dynamic_cast<Dialectric *>(m_material) != nullptr) {
  		Dialectric *d_mat = (Dialectric *)m_material;
  		// fresnel math -- copied from A4.cpp (should extract at some point)
  		glm::vec3 D = glm::normalize(a - b); // reverse of ray
  		double n1, n2;
  		glm::vec3 N = N_orig;
	  	if (glm::dot(D, N) <= 0.0) {
	  		// inside of the object (Assuming outside is air)
	  		n1 = d_mat->get_idx_ref();
	  		n2 = 1.0;
	  		N = -N;
	  	} else {
	  		// outside of the object
	  		n1 = 1.0;
	  		n2 = d_mat->get_idx_ref();
	  	}
	  	// Calculate proportion of light transmitted/reflected
	  	// Fresnel - Schlick's Approximation
	  	double R0 = glm::pow((n1-n2)/(n1+n2),2);
	  	double RThetai = R0 + (1.0-R0) * glm::pow(1.0 - glm::dot(D, N), 5);
	  	return RThetai * (1.0 - get_percent_absoption(a,b,N_orig, i));
	} else if (i->node->m_texture_map != NULL) {
		return 0.0;
	} else {
    	std::cout << "Unkown material type" << std::endl;
    	throw;
	}
}

double GeometryNode::get_percent_refraction(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &N, Intersection *i) {
	return 1.0 - (get_percent_absoption(a,b,N, i) + get_percent_reflection(a,b,N, i));
}

glm::vec3 GeometryNode::get_reflect_endpoint(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &N) {
	return (b-a) - 2.0f*N*glm::dot(N, b-a); // doesn't like double
}

glm::vec3 GeometryNode::get_refract_endpoint(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &N_orig) {
	if (dynamic_cast<Dialectric *>(m_material) == nullptr) {
		std::cout << "Call to GeometryNode::get_refract_endpoint on non-dialectric material." << std::endl;
    	throw;
	}
	Dialectric * d_mat = (Dialectric *)m_material;

	glm::vec3 D = glm::normalize(a - b); // reverse of ray
	glm::vec3 N = N_orig;
  	float n1, n2; // doesn't like double
  	if (glm::dot(D, N) <= 0.0) {
  		// inside of the object (Assuming outside is air)
  		n1 = d_mat->get_idx_ref();
  		n2 = 1.0;
  		N = -N;
  	} else {
  		// outside of the object
  		n1 = 1.0;
  		n2 = d_mat->get_idx_ref();
  	}

  	double EPSILON = 1.0/1024.0;
	float cos_thetai = glm::dot(glm::normalize(a - b), N); // doesn't like double
	double sqrt_term = 1 - (pow(n1, 2)/pow(n2,2))*(1 - pow(cos_thetai, 2));
	float sqrt_term_result; // doesn't like double
	if (sqrt_term < -EPSILON) { // Negative under sqrt -- total internal reflection
		return glm::vec3(0.0,0.0,0.0);
	} else if (sqrt_term >= -EPSILON && sqrt_term <= EPSILON) { // approx 0
		sqrt_term_result = 0.0; 
	} else {
		sqrt_term_result = glm::sqrt(sqrt_term);
	}
	glm::vec3 T = glm::normalize(b - a)*(n1/n2) - N * ((n1/n2)*cos_thetai + sqrt_term_result);
	return T + b;
}
