#include <glm/ext.hpp>
#include <functional>
#include <math.h>

#include "A4.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"

const uint MAX_HITS = 10;

const double EPSILON = 0.00000000001;
const glm::vec3 ZERO_VECTOR3 = glm::vec3(0.0,0.0,0.0);

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
	if (node->m_nodeType != NodeType::GeometryNode) {
		return Intersection();
	} else {
		GeometryNode *geo_node = (GeometryNode *)node;
		Intersection i = Intersection(geo_node->m_primitive->intersection(a, b), geo_node);
		i = i.t < EPSILON ? Intersection() : i;
		return i;
	}
}

Intersection recursive_intersect(glm::vec3 a, glm::vec3 b, SceneNode *node) {
	// find intersects from ray defined by (b-a) = a -----> b
	Intersection i = intersect(a, b, node);
	for (SceneNode * child: node->children) {
		Intersection iprime = recursive_intersect(a, b, child);
		if (!i.has_intersected || iprime.t < i.t || i.t < EPSILON) {
			i = iprime;
		}
	}
	return i;
}

glm::vec3 direct_light(const glm::vec3 &p, const glm::vec3 &N, Light *light, SceneNode *node) {
	if (!glm::dot(light->position - p, N) > 0) { // not on same side as light
		return glm::vec3(0.0,0.0,0.0);
	}
	Intersection intersection = recursive_intersect(p, light->position, node);
	if (intersection.has_intersected && intersection.t > 0 && intersection.t < 1) { // there's a shadow cast
		return glm::vec3(0.0,0.0,0.0);
	}
	return light->colour;
}

glm::vec3 get_color_of_intersection(Intersection intersection, glm::vec3 a, glm::vec3 b,
								    const glm::vec3 & ambient, const std::list<Light *> & lights,
								    uint max_hits, SceneNode * node) {
	// Node will be used to determine shadows
	// a---->b is ray
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
  	//double shininess = p_mat->get_shininess();

  	glm::vec3 col = ke + entrywise_multiply(kd, ambient);

  	glm::vec3 p = ray_point_at_parameter(a, b, intersection.t);
  	glm::vec3 N = intersection.node->m_primitive->get_normal_at_point(p);

  	if (!vector_equals(kd, ZERO_VECTOR3)) {
  		for (Light * light : lights) {
  			col += entrywise_multiply(kd, direct_light(p,N,light,node));
  		}
  	}

  	// DEAL WITH KS and reflections

	return col;
}

glm::mat4 screen_to_world(uint nx, uint ny,
					      const glm::vec3 & eye,
					      const glm::vec3 & view,
					      const glm::vec3 & up,
					      double fovy) {

	double d = glm::distance(eye, view);
	glm::mat4 T1 = glm::translate(glm::vec3(-(double)nx/2, -(double)ny/2, d));

	double height = 2.0 * d * glm::tan(fovy/2.0);
	//double width = (double)nx/(double)ny * height;
	double scale_factor = height/ny;
	glm::mat4 S2 = glm::scale(glm::vec3(-scale_factor, scale_factor, 1));

	glm::vec3 w = glm::normalize(view - eye);
	glm::vec3 u = glm::normalize(glm::cross(up, w));
	glm::vec3 v = glm::cross(w,u);
	glm::mat4 R3 = glm::mat4(
		glm::vec4(u,0),
		glm::vec4(v,0),
		glm::vec4(w,0),
		glm::vec4(0,0,0,1)
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
			glm::vec3 pixel = glm::vec3(S2W_transform * glm::vec4(x,y,0,1));
			ray_trace(x,y,w,h,eye,pixel,root,image,ambient,lights);
		}
	}

}

