#include <glm/ext.hpp>
#include <functional>
#include <math.h>
#include <cstdlib>

#include "A4.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include "Dialectric.hpp"

const uint MAX_HITS = 4;
const uint NUM_SAMPLES = 1;
const uint NUM_SAMPLES_EACH_DIR = (uint)glm::sqrt(NUM_SAMPLES);
const bool JITTERING = false;

const double EPSILON = 1.0/1024.0;
const glm::vec3 ZERO_VECTOR3 = glm::vec3(0.0,0.0,0.0);

double deg_to_rad(double deg) {
	return deg * 0.01745329252; // pi / 180
}

bool vector_equals(const glm::vec3 &a, const glm::vec3 &b) {
	if (
		glm::abs(a.x - b.x) < EPSILON &&
		glm::abs(a.y - b.y) < EPSILON &&
		glm::abs(a.z - b.z) < EPSILON
	) return true;
	return false;
}

// Forward declare
glm::vec3 get_ray_color(glm::vec3 a, glm::vec3 b, const glm::vec3 & ambient, const std::list<Light *> & lights,
			  		    uint hits_allowed, SceneNode * node, uint pixelx, uint pixely, uint imagew, uint imageh, bool is_reflection);
//


glm::vec3 get_background_pixel (uint x, uint y, uint w, uint h, bool is_reflection) {
	// Was just playing around, but this makes a really cool pattern
	// different numbers give different patterns, with primes being
	// the most interesting
	// If the prime is low, you get extremely cool patterns
	// For larger primes, you get a random-star effect

	static const uint PRIME = 811;
	if ((x + y)*(x - y) % PRIME == 0 && x < y && !is_reflection) return glm::vec3(1.0);
	if ((w + h - x - y)*(w - x - h + y) % PRIME == 0 && y < x && !is_reflection) return glm::vec3(1.0);

	return glm::vec3(
		0.5 - 0.4 * (double)x / (double)w,
		0.4 - 0.35 * (double)y / (double)h,
		1.0 - 0.8 * (double)y / (double)h
	);
}

void set_pixel (glm::vec3 color, uint x, uint y, Image & image) {
	image(x,y,0) = color[0];
  	image(x,y,1) = color[1];
  	image(x,y,2) = color[2];
}

glm::vec3 ray_point_at_parameter(const glm::vec3 & a, const glm::vec3 & b, double t) {
	return a + t*(b-a);
}

glm::vec3 entrywise_multiply(const glm::vec3 &a, const glm::vec3 &b) {
	return glm::vec3(a.x * b.x, a.y * b.y , a.z * b.z);
}


Intersection *intersect(glm::vec3 a, glm::vec3 b, SceneNode *node,
					   Intersection * prev_intersection) {
	if (node->m_nodeType != NodeType::GeometryNode) return new Intersection();
	GeometryNode *geo_node = (GeometryNode *)node;
	Intersection *i = geo_node->m_primitive->intersection(a, b, prev_intersection);
	i->node = geo_node;
	
	if (i->has_intersected) {
		i->local_intersection = ray_point_at_parameter(a,b,i->t);
	}
	return i;
}

Intersection *recursive_intersect(glm::vec3 a, glm::vec3 b, SceneNode *node, glm::mat4 parent_invtrans,
								 Intersection * prev_intersection) {
	// find intersects from ray defined by (b-a) = a -----> b

	// already applied parent_trans recursively
	a = glm::vec3(node->invtrans * glm::vec4(a,1));
	b = glm::vec3(node->invtrans * glm::vec4(b,1));

	glm::mat4 invtrans =  node->invtrans * parent_invtrans;

	Intersection *i = intersect(a, b, node, prev_intersection);
	i->invtrans = invtrans;
	
	for (SceneNode * child: node->children) {
		Intersection *iprime = recursive_intersect(a, b, child, invtrans, prev_intersection);
		if (!i->has_intersected || iprime->t < i->t) {
			delete i;
			i = iprime;
		} else {
			delete iprime;
		}
	}
	return i;
}

double cast_shadow_ray(glm::vec3 hit, glm::vec3 norm, glm::vec3 light_pos, SceneNode *node, Intersection * prev_intersection) {
	// true if shadow cast
	if (glm::dot(light_pos-hit, norm) <= EPSILON) { // not on same side as light
		return 1.0;
	}
	Intersection *intersection = recursive_intersect(hit, light_pos, node, glm::mat4(), prev_intersection);
	if (intersection->has_intersected && intersection->t > 0.0 && intersection->t < 1.0) { // there's a shadow cast
		delete intersection;
		return 1.0;
	}
	delete intersection;
	return 0.0;
}

glm::vec3 direct_light(const glm::vec3 &p, const glm::vec3 &N, Light *light, SceneNode *node, Intersection * prev_intersection) {
	double percent_shadow = cast_shadow_ray(p, N, light->position, node, prev_intersection);

	glm::vec3 surface_to_light = light->position - p;
	double dist = glm::length(surface_to_light);

	return (1.0 - percent_shadow) * light->colour * (glm::dot(glm::normalize(surface_to_light), N)) / 
		   (light->falloff[0] + light->falloff[1]*dist + light->falloff[2]*dist*dist);
}

glm::vec3 get_reflected_color(glm::vec3 a, glm::vec3 p, glm::vec3 N, const glm::vec3 & ambient,
						      const std::list<Light *> & lights,
							  uint hits_allowed, SceneNode * node,
							  Intersection * prev_intersection,
							  uint x, uint y, uint w, uint h) {
	if (hits_allowed <= 0) return glm::vec3();
	glm::vec3 r = (p-a) - 2*N*glm::dot(N, p-a);
	return get_ray_color(p, r + p, ambient, lights, hits_allowed, node, x, y, w, h, true);
}

glm::vec3 get_transmitted_color(glm::vec3 a, glm::vec3 p, glm::vec3 N, const glm::vec3 & ambient,
						      const std::list<Light *> & lights,
							  uint hits_allowed, SceneNode * node,
							  Intersection * prev_intersection,
							  uint x, uint y, uint w, uint h) {
	return glm::vec3(0.0);
}

glm::vec3 get_color_of_intersection_phong(Intersection *intersection, PhongMaterial *p_mat,
										  glm::vec3 a, glm::vec3 b,
									      const glm::vec3 & ambient, const std::list<Light *> & lights,
									      uint hits_allowed, SceneNode * node,
									      uint pixelx, uint pixely,
										  uint imagew, uint imageh) {

	const static double AMBIENT_DAMPING_FACTOR = 0.4;
	const static double REFLECTION_DAMPING_FACTOR = 0.5;

	glm::vec3 ke = glm::vec3(0.0,0.0,0.0); // The objects are non-emittive

  	glm::vec3 kd = p_mat->get_kd();
  	glm::vec3 ks = p_mat->get_ks();
  	double shininess = p_mat->get_shininess();

  	glm::vec3 col = ke + AMBIENT_DAMPING_FACTOR * entrywise_multiply(kd, ambient); // ka = kd

  	glm::vec3 p = ray_point_at_parameter(a, b, intersection->t);
	glm::vec3 p_model = intersection->local_intersection;
  	glm::vec3 N_model = intersection->node->m_primitive->get_normal_at_point(p_model, intersection);
  	glm::vec3 N = glm::vec3(glm::transpose(intersection->invtrans) * glm::vec4(N_model, 1.0));
  	if (!vector_equals(N, ZERO_VECTOR3)) {
  		N = glm::normalize(N);
  	}

  	if (!vector_equals(kd, ZERO_VECTOR3)) {
  		for (Light * light : lights) {
  			col += entrywise_multiply(kd, direct_light(p, N, light,node, intersection));
  		}
  	}
	double percent_shadow;
  	if (!vector_equals(ks, ZERO_VECTOR3)) {
  		// direct light reflection
	  	for (Light * light : lights) {
	  		percent_shadow = cast_shadow_ray(p, N, light->position, node, intersection);
	  		glm::vec3 l = light->position - p;
	  		glm::vec3 r = -l + 2.0*(glm::dot(l,N))*N;
	  		col += (1.0 - percent_shadow) * entrywise_multiply(ks, glm::pow(glm::dot(glm::normalize(r), glm::normalize(a - p)), shininess) * light->colour);
	  	}

  		// Do a reflection
  		glm::vec3 ref_col = get_reflected_color(a, p, N, ambient, lights, hits_allowed - 1, node, intersection, pixelx, pixely, imagew, imageh);
  		col += REFLECTION_DAMPING_FACTOR * entrywise_multiply(ks, ref_col);
  	}

	return col;
}

glm::vec3 get_color_of_intersection_dialectric(Intersection *intersection, Dialectric* d_mat,
											   glm::vec3 a, glm::vec3 b,
										       const glm::vec3 & ambient, const std::list<Light *> & lights,
										       uint hits_allowed, SceneNode * node,
										       uint pixelx, uint pixely,
											   uint imagew, uint imageh) {
	glm::vec3 p = ray_point_at_parameter(a, b, intersection->t);
	glm::vec3 p_model = intersection->local_intersection;
  	glm::vec3 N_model = intersection->node->m_primitive->get_normal_at_point(p_model, intersection);
  	glm::vec3 N = glm::vec3(glm::transpose(intersection->invtrans) * glm::vec4(N_model, 1.0));

  	glm::vec3 D = b - a;
  	double n1, n2;
  	if (glm::dot(D, N) >= 0) {
  		// inside of the object (Assuming outside is air)
  		n1 = d_mat->get_idx_ref();
  		n2 = 1.0;

  	} else {
  		// outside of the object
  		n1 = 1.0;
  		n2 = d_mat->get_idx_ref();
  	}

  	glm::vec3 ref_col = get_reflected_color(a, p, N, ambient, lights, hits_allowed - 1, node, intersection, pixelx, pixely, imagew, imageh);
  	glm::vec3 trans_col = get_transmitted_color(a, p, N, ambient, lights, hits_allowed - 1, node, intersection, pixelx, pixely, imagew, imageh);
  	
  	// Calculate proportion of light transmitted/reflected

  	return 0.6 * ref_col + trans_col;

}

glm::vec3 get_color_of_intersection(Intersection *intersection, glm::vec3 a, glm::vec3 b,
								    const glm::vec3 & ambient, const std::list<Light *> & lights,
								    uint hits_allowed, SceneNode * node,
								    uint pixelx, uint pixely,
									uint imagew, uint imageh) {
	// a---->b is ray
	// hits_allowed will decrease if tracing reflections

	if (!intersection->has_intersected) {
		std::cout << "set_pixel called with !intersect.has_intersected" << std::endl;
		delete intersection;
		throw;
	}

	Material *mat = intersection->node->m_material;
	if (dynamic_cast<PhongMaterial*>(mat) != nullptr) {	
  		return get_color_of_intersection_phong(intersection, (PhongMaterial *)mat, a, b, ambient, lights,
											   hits_allowed, node, pixelx, pixely, imagew, imageh);
  	} else if (dynamic_cast<Dialectric *>(mat) != nullptr) {
  		return get_color_of_intersection_dialectric(intersection, (Dialectric *)mat, a, b, ambient, lights,
											        hits_allowed, node, pixelx, pixely, imagew, imageh);
	} else {
    	std::cout << "Unkown material type" << std::endl;
    	throw;
	}
}

glm::vec3 get_ray_color(glm::vec3 a, glm::vec3 b, 
						const glm::vec3 & ambient, const std::list<Light *> & lights,
						uint hits_allowed, SceneNode * node,
						uint pixelx, uint pixely,
						uint imagew, uint imageh, bool is_reflection) {
	Intersection *intersection = recursive_intersect(a, b, node, glm::mat4(), NULL);
	glm::vec3 color;
	if (!intersection->has_intersected) {
		color = get_background_pixel(pixelx,pixely,imagew,imageh, is_reflection);
	} else {
		color = get_color_of_intersection(intersection, a, b, ambient, lights, hits_allowed, node, pixelx, pixely, imagew, imageh);
	}
	delete intersection;
	return color;
}

glm::mat4 screen_to_world(uint nx, uint ny,
					      const glm::vec3 & eye,
					      const glm::vec3 & view,
					      const glm::vec3 & up,
					      double fovy) {

	double d = glm::distance(eye, view);
	glm::mat4 T1 = glm::translate(glm::vec3(-(double)nx/2.0, -(double)ny/2.0, d));

	double height = 2.0 * d * glm::tan(deg_to_rad(fovy)/2.0);
	//double width = (double)nx/(double)ny * height;
	double scale_factor = height/ny;
	glm::mat4 S2 = glm::scale(glm::vec3(-scale_factor, scale_factor, 1.0));

	glm::vec3 w = glm::normalize(view - eye);
	glm::vec3 u = glm::normalize(glm::cross(up, w));
	glm::vec3 v = glm::cross(u,w);
	glm::mat4 R3 = glm::mat4(
		glm::vec4(u,0.0),
		glm::vec4(v,0.0),
		glm::vec4(w,0.0),
		glm::vec4(0.0,0.0,0.0,1.0)
	);

	glm::mat4 T4 = glm::translate(eye);

	return T4 * R3 * S2 * T1; 
}

void ray_trace(uint x, uint y, uint w, uint h,
			   glm::vec3 eye, const glm::mat4 & S2W_transform, SceneNode *node,
			   Image & image, const glm::vec3 & ambient, const std::list<Light *> & lights) {

	glm::vec3 col;

	// uniform sample - Anti Aliasing
	glm::vec3 color = glm::vec3(0.0);
	for (int l = 0; l < NUM_SAMPLES_EACH_DIR; l++) {
		for (int k = 0; k < NUM_SAMPLES_EACH_DIR; k++) {
			glm::vec3 pixel;
			if (JITTERING) {
				double z1 = (double)rand()/(RAND_MAX);
				double z2 = (double)rand()/(RAND_MAX);
				pixel = glm::vec3(S2W_transform * glm::vec4(
					(double)x + (k + z1)/((double)NUM_SAMPLES_EACH_DIR),
					(double)y + (l + z2)/((double)NUM_SAMPLES_EACH_DIR),
					0.0, 1.0
				));
			} else {
				pixel = glm::vec3(S2W_transform * glm::vec4(
					(double)x + (2.0 * (double)k + 1)/(2.0*(double)NUM_SAMPLES_EACH_DIR),
					(double)y + (2.0 * (double)l + 1)/(2.0*(double)NUM_SAMPLES_EACH_DIR),
					0.0, 1.0
				));
			}
			color += get_ray_color(eye, pixel, ambient, lights, MAX_HITS, node, x, y, w, h, false);
		}
	}
	color = color / (NUM_SAMPLES_EACH_DIR * NUM_SAMPLES_EACH_DIR);
	set_pixel(color, x, y, image);
}

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...

  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	glm::mat4 S2W_transform = screen_to_world(w,h,eye,view,up,fovy);

	const uint TOTAL_PIXELS = h * w ;
	double current_milestone = 0.00;

	std::cout << "Current renderring progress:" << std::endl;

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			ray_trace(x,y,w,h,eye,S2W_transform,root,image,ambient,lights);

			// progress indicator
			if ((y * w + x + 1) / (double)TOTAL_PIXELS >= current_milestone - EPSILON ) {
				std::cout << "[";
				for (int i = 0; i < int(current_milestone*20); i++) {
					std::cout << "-";
				}
				for (int i = 0; i < 20 - int(current_milestone*20); i++) {
					std::cout << " ";
				}
				std::cout << "] ";
				uint amount = int(current_milestone * 100.0);
				amount += 2;
				amount -= amount % 5; // round to nearest 5
				std::cout << amount << " %\r";
    			std::cout.flush();
				current_milestone += 0.05;
			}
		}
	}
	// used \r in progress indicator
	std::cout << std::endl;
}

