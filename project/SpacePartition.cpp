#include "SpacePartition.hpp"
#include "GeometryNode.hpp"
#include <glm/ext.hpp>
#include "Primitive.hpp"

SpacePartition::SpacePartition():
	min(glm::vec3()),
	max(glm::vec3()),
	root(NULL),
	depth(0),
	nodes(std::vector<GeometryNode *>()),
	is_partitioned(false)
{
	for (uint i = 0; i < 8; i++) children[i] = NULL;
}

SpacePartition::SpacePartition(glm::vec3 min, glm::vec3 max, SceneNode * root, uint depth):
	min(min),
	max(max),
	root(root),
	depth(depth),
	nodes(std::vector<GeometryNode *>()),
	is_partitioned(false)
{
	for (uint i = 0; i < 8; i++) children[i] = NULL;
	fill();
}

SpacePartition::~SpacePartition() {
	for (uint i = 0; i < 8; i++) { // recursively delete each child
		if (children[i] != NULL) {
			delete children[i];
			children[i] = NULL;
		}
	}
}

glm::vec3 SpacePartition::ray_point_at_parameter(const glm::vec3 & a, const glm::vec3 & b, double t) {
	return a + t*(b-a);
}

glm::vec3 SpacePartition::get_bot_left_corner(SceneNode * node) {
	glm::vec3 current_corner;
	bool current_corner_set = false;

	for (SceneNode * child : node->children) {
		glm::vec3 child_blc = get_bot_left_corner(child);
		if (!current_corner_set) {
			current_corner = child_blc;
			current_corner_set = true;
			continue;
		}
		if (child_blc.x < current_corner.x) current_corner.x = child_blc.x;
		if (child_blc.y < current_corner.y) current_corner.y = child_blc.y;
		if (child_blc.z < current_corner.z) current_corner.z = child_blc.z;
	}

	if (node->m_nodeType == NodeType::GeometryNode) {
		GeometryNode * gnode = (GeometryNode *)node;
		glm::vec3 node_bb_blc = gnode->get_bb_bottom_left_corner();
		node_bb_blc = glm::vec3(glm::vec4(node_bb_blc,1.0) * gnode->get_squashed_trans()); // translate to world
		if (!current_corner_set) {
			current_corner = node_bb_blc;
		}
		if (node_bb_blc.x < current_corner.x) current_corner.x = node_bb_blc.x;
		if (node_bb_blc.y < current_corner.y) current_corner.y = node_bb_blc.y;
		if (node_bb_blc.z < current_corner.z) current_corner.z = node_bb_blc.z;
	}
	return current_corner;

}

glm::vec3 SpacePartition::get_top_right_corner(SceneNode * node) {
	glm::vec3 current_corner;
	bool current_corner_set = false;

	for (SceneNode * child : node->children) {
		glm::vec3 child_trc = get_top_right_corner(child);
		if (!current_corner_set) {
			current_corner = child_trc;
			current_corner_set = true;
			continue;
		}
		if (child_trc.x > current_corner.x) current_corner.x = child_trc.x;
		if (child_trc.y > current_corner.y) current_corner.y = child_trc.y;
		if (child_trc.z > current_corner.z) current_corner.z = child_trc.z;
	}

	if (node->m_nodeType == NodeType::GeometryNode) {
		GeometryNode * gnode = (GeometryNode *)node;
		glm::vec3 node_bb_trc = gnode->get_bb_top_right_corner();
		node_bb_trc = glm::vec3(glm::vec4(node_bb_trc,1.0) * gnode->get_squashed_trans()); // translate to world
		if (!current_corner_set) {
			current_corner = node_bb_trc;
		}
		if (node_bb_trc.x > current_corner.x) current_corner.x = node_bb_trc.x;
		if (node_bb_trc.y > current_corner.y) current_corner.y = node_bb_trc.y;
		if (node_bb_trc.z > current_corner.z) current_corner.z = node_bb_trc.z;
	}
	return current_corner;
}

bool SpacePartition::box_intersect(glm::vec3 b1, glm::vec3 t1, glm::vec3 b2, glm::vec3 t2) {
	//b1 is bot left corner of box 1, t1 is top left corner of box 1
	//b2 is bot left corner of box 2, t2 is top left corner of box 2
	// intersection if a corner of one box is inside the other box
	if (b1.x >= b2.x && b1.x <= t2.x && b1.y >= b2.y && b1.y <= t2.y && b1.z >= b2.z && b1.z <= t2.z) return true;
	if (t1.x >= b2.x && t1.x <= t2.x && t1.y >= b2.y && t1.y <= t2.y && t1.z >= b2.z && t1.z <= t2.z) return true;
	if (b2.x >= b1.x && b2.x <= t1.x && b2.y >= b1.y && b2.y <= t1.y && b2.z >= b1.z && b2.z <= t1.z) return true;
	if (t2.x >= b1.x && t2.x <= t1.x && t2.y >= b1.y && t2.y <= t1.y && t2.z >= b1.z && t2.z <= t1.z) return true;
	return false;
}

void SpacePartition::recursive_fill(SceneNode * node) {
	for (SceneNode * child : node->children) {
		recursive_fill(child);
	}
	if (node->m_nodeType != NodeType::GeometryNode) return;
	GeometryNode * gnode = (GeometryNode *)node;
	glm::vec3 blc = glm::vec3(glm::vec4(gnode->get_bb_bottom_left_corner(),1.0) * gnode->get_squashed_trans());
	glm::vec3 trc = glm::vec3(glm::vec4(gnode->get_bb_top_right_corner(),1.0) * gnode->get_squashed_trans());
	if (box_intersect(blc, trc, min, max)) nodes.push_back(gnode);
}

// Will fill based on nodes whose bb intersects with min/max box.
// If more than MAX_NODES nodes, will subdivide
void SpacePartition::fill() {
	recursive_fill(root);
	if (depth >= 5) return;
	if (nodes.size() > MAX_NODES) {
		is_partitioned = true;
		// subdivide:
		glm::vec3 midpoint = (max + min)/2.0;
		// bot left front
		children[0] = new SpacePartition(min, midpoint, root, depth + 1);
		// bot right front
		children[1] = new SpacePartition(glm::vec3(midpoint.x, min.y, min.z), 
										 glm::vec3(max.x, midpoint.y, midpoint.z), root, depth + 1);
		// top left front
		children[2] = new SpacePartition(glm::vec3(min.x, midpoint.y, min.z), 
										 glm::vec3(midpoint.x, max.y, midpoint.z), root, depth + 1);
		// top right front
		children[3] = new SpacePartition(glm::vec3(midpoint.x, midpoint.y, min.z), 
										 glm::vec3(max.x, max.y, midpoint.z), root, depth + 1);
		// bot left back
		children[4] = new SpacePartition(glm::vec3(min.x, min.y, midpoint.z), 
										 glm::vec3(midpoint.x, midpoint.y, max.z), root, depth + 1);
		// bot right back
		children[5] = new SpacePartition(glm::vec3(midpoint.x, min.y, midpoint.z), 
										 glm::vec3(max.x, midpoint.y, max.z), root, depth + 1);
		// top left back
		children[6] = new SpacePartition(glm::vec3(min.x, midpoint.y, midpoint.z), 
										 glm::vec3(midpoint.x, max.y, max.z), root, depth + 1);
		// top right back
		children[7] = new SpacePartition(midpoint, max, root, depth + 1);
	}
}

Intersection *SpacePartition::local_intersect(glm::vec3 a, glm::vec3 b, GeometryNode *node) {
	a = glm::vec3(node->get_squashed_invtrans() * glm::vec4(a, 1.0));
	b = glm::vec3(node->get_squashed_invtrans() * glm::vec4(b, 1.0));
	Intersection *i = node->m_primitive->intersection(a, b, NULL);
	i->node = node;
	if (i->has_intersected) i->local_intersection = ray_point_at_parameter(a,b,i->t);
	return i;
}

Intersection *SpacePartition::intersect(glm::vec3 a, glm::vec3 b) {
	if (is_partitioned) {
		Intersection *i = children[0]->intersect(a,b);
		for (int j = 1; j < 8; j++) {
			Intersection *iprime = children[j]->intersect(a,b);
			if (!i->has_intersected || iprime->t < i->t) {
				delete i;
				i = iprime;
			} else {
				delete iprime;
			}

		}
		return i;
	} else {
		if (nodes.empty()) {
			return new Intersection();
		}
		Intersection *i = local_intersect(a,b,nodes.at(0));
		for (int j = 1; j < nodes.size(); j++) {
			Intersection *iprime = local_intersect(a,b,nodes.at(j));
			if (!i->has_intersected || iprime->t < i->t) {
				delete i;
				i = iprime;
			} else {
				delete iprime;
			}
		}
		return i;
	}
}

//
void SpacePartition::initialize(SceneNode *root) {
	this->root = root;
	this->min = get_bot_left_corner(root);
	this->max = get_top_right_corner(root);
	fill();
}
