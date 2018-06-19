#include <glm/ext.hpp>
#include <functional>
#include <math.h>

#include "A4.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"

const uint MAX_HITS = 1;

const double EPSILON = 0.0000000001;
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

void set_background_pixel (uint x, uint y, uint w, uint h, Image & image) {
	// Red: increasing from top to bottom
	image(x, y, 0) = (double)y / h;
	// Green: increasing from left to right
	image(x, y, 1) = (double)x / w;
	// Blue: in lower-left and upper-right corners
	image(x, y, 2) = ((y < h/2 && x < w/2)
					|| (y >= h/2 && x >= w/2)) ? 1.0 : 0.0;
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


Intersection intersect(glm::vec3 a, glm::vec3 b, SceneNode *node) {
	static const double MIN_INT_DIST = 0.001;
	if (node->m_nodeType != NodeType::GeometryNode) {
		return Intersection();
	} else {
		GeometryNode *geo_node = (GeometryNode *)node;
		Intersection i = Intersection(geo_node->m_primitive->intersection(a, b), geo_node);
		i = i.t < MIN_INT_DIST ? Intersection() : i;
		return i;
	}
}

Intersection recursive_intersect(glm::vec3 a, glm::vec3 b, SceneNode *node) {
	// find intersects from ray defined by (b-a) = a -----> b
	Intersection i = intersect(a, b, node);
	for (SceneNode * child: node->children) {
		Intersection iprime = recursive_intersect(a, b, child);
		if (!i.has_intersected || iprime.t < i.t) {
			i = iprime;
		}
	}
	return i;
}

glm::vec3 direct_light(const glm::vec3 &p, const glm::vec3 &N, Light *light, SceneNode *node) {
	glm::vec3 light_to_surface = light->position - p;

	if (!glm::dot(light_to_surface, N) > 0) { // not on same side as light
		return glm::vec3(0.0,0.0,0.0);
	}
	
	Intersection intersection = recursive_intersect(p, light->position, node);
	if (intersection.has_intersected && intersection.t > 0.0 && intersection.t < 1.0) { // there's a shadow cast
		return glm::vec3(0.0,0.0,0.0);
	}

	double dist = glm::length(light_to_surface);
	static const double C1 = 1.0;
	static const double C2 = 0.0;
	static const double C3 = 0.0;

	return light->colour * (glm::dot(glm::normalize(light_to_surface), N)) / (C1 + C2*dist + C3*dist*dist);
}

/*
glm::vec3 get_reflected_ray_endpoint (glm::vec3 a, glm::vec3 b, glm::vec3 N) {
	// make new ray b-->q with angle(N, q-b) == angle(N, a-b)
	// https://math.stackexchange.com/questions/13261/how-to-get-a-reflection-vector
	return (b-a) - 2.0 * glm::dot(b-a, N) * N;
}
*/

glm::vec3 get_reflected_color(glm::vec3 a, glm::vec3 p, glm::vec3 N,
						      double shininess, const glm::vec3 & ambient,
						      const std::list<Light *> & lights,
							  uint hits_allowed, SceneNode * node) {
  	glm::vec3 colour = glm::vec3(0.0,0.0,0.0);
  	for (Light * light : lights) {
  		Intersection intersection = recursive_intersect(p, light->position, node);
		if (intersection.has_intersected && intersection.t > 0.0 && intersection.t < 1.0) { // there's a shadow cast
			continue;
		}
  		glm::vec3 l = light->position - p;
  		glm::vec3 r = -l + 2.0*(glm::dot(l,N))*N;

  		colour += glm::pow(glm::dot(glm::normalize(r), glm::normalize(a - p)), shininess) * light->colour;
  	}
  	return colour;
  	/*
  	glm::vec3 q = get_reflected_ray_endpoint(a, p, N);
  	Intersection intersection = recursive_intersect(p, q, node);
  	if (!intersection.has_intersected) {
		return glm::vec3(0.0,0.0,0.0); // return background?
	} else {
  		return get_color_of_intersection(intersection, p, q, ambient, lights, hits_allowed, node);
	}*/
}

glm::vec3 get_color_of_intersection(Intersection intersection, glm::vec3 a, glm::vec3 b,
								    const glm::vec3 & ambient, const std::list<Light *> & lights,
								    uint hits_allowed, SceneNode * node) {
	// Node will be used to determine shadows
	// a---->b is ray
	// hits_allowed will decrease if tracing reflections
	if (!intersection.has_intersected) {
		std::cout << "set_pixel called with !intersect.has_intersected" << std::endl;
		throw;
	}

	Material *mat = intersection.node->m_material;
	if (dynamic_cast<PhongMaterial*>(mat) == nullptr) {	
    	std::cout << "Unkown material type" << std::endl;
  	}

  	PhongMaterial *p_mat = (PhongMaterial *)mat;

  	glm::vec3 ke = glm::vec3(0.0,0.0,0.0); // The objects are non-emittive

  	glm::vec3 kd = p_mat->get_kd();
  	glm::vec3 ks = p_mat->get_ks();
  	double shininess = p_mat->get_shininess();

  	glm::vec3 col = ke + entrywise_multiply(kd, ambient);

  	glm::vec3 p = ray_point_at_parameter(a, b, intersection.t);
  	glm::vec3 N = intersection.node->m_primitive->get_normal_at_point(p);

  	if (!vector_equals(kd, ZERO_VECTOR3)) {
  		for (Light * light : lights) {
  			col += entrywise_multiply(kd, direct_light(p,N,light,node));
  		}
  	}

  	if (!vector_equals(ks, ZERO_VECTOR3) && hits_allowed > 0) {
  		// Do a reflection
  		glm::vec3 ref_col = get_reflected_color(a, p, N, shininess, ambient, lights, hits_allowed - 1, node);
  		col += entrywise_multiply(ks, ref_col);
  	}

	return col;
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
			   glm::vec3 eye, glm::vec3 pixel, SceneNode *node,
			   Image & image, const glm::vec3 & ambient, const std::list<Light *> & lights) {

	glm::vec3 col;

	Intersection intersection = recursive_intersect(eye, pixel, node);

	if (!intersection.has_intersected) {
		set_background_pixel(x,y,w,h,image);
	} else {
		// deal with intersection
		glm::vec3 color = get_color_of_intersection(intersection, eye, pixel, ambient, lights, MAX_HITS, node);
		set_pixel(color, x, y, image);
	}

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

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			glm::vec3 pixel = glm::vec3(S2W_transform * glm::vec4((double)x,(double)y,0.0,1.0));
			ray_trace(x,y,w,h,eye,pixel,root,image,ambient,lights);
		}
	}

}

