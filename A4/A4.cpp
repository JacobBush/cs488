#include <glm/ext.hpp>
#include <functional>
#include <math.h>

#include "A4.hpp"
#include "GeometryNode.hpp"

void set_background_pixel (int x, int y, int w, int h, Image & image) {
	// Red: increasing from top to bottom
	image(x, y, 0) = (double)y / h;
	// Green: increasing from left to right
	image(x, y, 1) = (double)x / w;
	// Blue: in lower-left and upper-right corners
	image(x, y, 2) = ((y < h/2 && x < w/2)
					|| (y >= h/2 && x >= w/2)) ? 1.0 : 0.0;
}

/*
std::function<glm::vec3 (int t)> make_ray(const glm::vec3 & eye, const glm::vec3 & pixel) {
	return [=](int t) -> glm::vec3 {
		return eye + t*(pixel - eye);
	};
}
*/

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


double intersect(glm::vec3 eye, glm::vec3 point, SceneNode *node) {
	// return of NAN if no intersect
	if (node->m_nodeType != NodeType::GeometryNode) return nan("");
	return ((GeometryNode *)node)->m_primitive->intersection(eye, point);
}

//void recursive_intersect(std::function<glm::vec3 (int t)> ray, SceneNode *node) {
double recursive_intersect(glm::vec3 eye, glm::vec3 point, SceneNode *node) {
	// return of NAN if no intersect
	double t = intersect(eye, point, node);
	for (SceneNode * child: node->children) {
		double tprime = recursive_intersect(eye, point, child);
		if (isnan(t)) {
			t = tprime;
		} else if (isnan(tprime)) {
			// t = t
		} else {
			t = glm::min(t, tprime);
		}
	}
	//if (t == NAN) std::cout << "NAN" << std::endl;
	return t;
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
			//std::cout << pixel.x << ", " <<  pixel.y << ", " <<pixel.z << std::endl;
			//std::function<glm::vec3 (int t)> ray = make_ray(eye, pixel);
			double intersection = recursive_intersect(eye, pixel, root);
			if (isnan(intersection)) {
				// We did not intersect with any objects
				set_background_pixel(x,y,w,h,image);
			} else {
				// deal with material
			}
		}
	}

}

