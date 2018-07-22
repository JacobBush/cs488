#include "SpacePartition.hpp"
#include "GeometryNode.hpp"
#include <glm/ext.hpp>
#include "Primitive.hpp"

SpacePartition::SpacePartition():
	posn(glm::vec3()),
	size(0.0),
	root(NULL),
	depth(0),
	nodes(std::vector<GeometryNode *>()),
	is_partitioned(false)
{
	for (uint i = 0; i < 8; i++) children[i] = NULL;
}

SpacePartition::SpacePartition(glm::vec3 posn, double size, std::vector<GeometryNode *> parent_nodes, uint depth):
	posn(posn),
	size(size),
	root(root),
	depth(depth),
	nodes(std::vector<GeometryNode *>()),
	is_partitioned(false)
{
	for (uint i = 0; i < 8; i++) children[i] = NULL;
	fill(parent_nodes);
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
		current_corner.x = glm::min(current_corner.x, child_blc.x);
		current_corner.y = glm::min(current_corner.y, child_blc.y);
		current_corner.z = glm::min(current_corner.z, child_blc.z);
	}

	if (node->m_nodeType == NodeType::GeometryNode) {
		GeometryNode * gnode = (GeometryNode *)node;
		glm::vec3 node_bb_blc = gnode->get_bb_bottom_left_corner();
		node_bb_blc = glm::vec3(glm::vec4(node_bb_blc,1.0) * gnode->get_squashed_trans()); // translate to world
		if (!current_corner_set) {
			current_corner = node_bb_blc;
		}
		current_corner.x = glm::min(current_corner.x, node_bb_blc.x);
		current_corner.y = glm::min(current_corner.y, node_bb_blc.y);
		current_corner.z = glm::min(current_corner.z, node_bb_blc.z);
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
		current_corner.x = glm::max(current_corner.x, child_trc.x);
		current_corner.y = glm::max(current_corner.y, child_trc.y);
		current_corner.z = glm::max(current_corner.z, child_trc.z);
	}

	if (node->m_nodeType == NodeType::GeometryNode) {
		GeometryNode * gnode = (GeometryNode *)node;
		glm::vec3 node_bb_trc = gnode->get_bb_top_right_corner();
		node_bb_trc = glm::vec3(glm::vec4(node_bb_trc,1.0) * gnode->get_squashed_trans()); // translate to world
		if (!current_corner_set) {
			current_corner = node_bb_trc;
		}
		current_corner.x = glm::max(current_corner.x, node_bb_trc.x);
		current_corner.y = glm::max(current_corner.y, node_bb_trc.y);
		current_corner.z = glm::max(current_corner.z, node_bb_trc.z);
	}
	return current_corner;
}

bool SpacePartition::box_intersect(glm::vec3 b1, glm::vec3 t1, glm::vec3 b2, glm::vec3 t2) {
	//b1 is bot left corner of box 1, t1 is top left corner of box 1
	//b2 is bot left corner of box 2, t2 is top left corner of box 2
	// intersection if a corner of one box is inside the other box
return (((b1.x <= b2.x && b2.x <= t1.x) || (b2.x <= b1.x && b1.x <= t2.x)) &&
       ((b1.y <= b2.y && b2.y <= t1.y) || (b2.y <= b1.y && b1.y <= t2.y)) &&
       ((b1.z <= b2.z && b2.z <= t1.z) || (b2.z <= b1.z && b1.z <= t2.z)));
}

void SpacePartition::partition() {
	// if (depth >= 10) return; // hopefully this is small enough for edge cases, large enough otherwise
	if (nodes.size() > MAX_NODES) { // copy-pasted
		is_partitioned = true;
		// subdivide:
		double half = size/2.0;
		// bot left front
		children[0] = new SpacePartition(posn + glm::vec3(0.0, 0.0, 0.0), half, nodes, depth + 1);
		// bot right front
		children[1] = new SpacePartition(posn + glm::vec3(half, 0.0, 0.0), half, nodes, depth + 1);
		// top left front
		children[2] = new SpacePartition(posn + glm::vec3(0.0, half, 0.0), half, nodes, depth + 1);
		// top right front
		children[3] = new SpacePartition(posn + glm::vec3(half, half, 0.0), half, nodes, depth + 1);
		// bot left back
		children[4] = new SpacePartition(posn + glm::vec3(0.0, 0.0, half), half, nodes, depth + 1);
		// bot right back
		children[5] = new SpacePartition(posn + glm::vec3(half, 0.0, half), half, nodes, depth + 1);
		// top left back
		children[6] = new SpacePartition(posn + glm::vec3(0.0, half, half), half, nodes, depth + 1);
		// top right back
		children[7] = new SpacePartition(posn + glm::vec3(half, half, half), half, nodes, depth + 1);
	}
}

// Will fill based on nodes whose bb intersects with min/max box.
// If more than MAX_NODES nodes, will subdivide
void SpacePartition::fill(std::vector<GeometryNode *> parent_nodes) {
	for (GeometryNode * node : parent_nodes) {
		glm::vec3 blc = glm::vec3(glm::vec4(node->get_bb_bottom_left_corner(),1.0) * node->get_squashed_trans());
		glm::vec3 trc = glm::vec3(glm::vec4(node->get_bb_top_right_corner(),1.0) * node->get_squashed_trans());
		if (box_intersect(blc, trc, posn, posn + glm::vec3(size))) nodes.push_back(node);
	}
	partition();
}

void SpacePartition::recursive_fill_root(SceneNode * node) {
	for (SceneNode * child : node->children) {
		recursive_fill_root(child);
	}
	if (node->m_nodeType != NodeType::GeometryNode) return;
	nodes.push_back((GeometryNode *)node);
}


void SpacePartition::fill_root() {
	recursive_fill_root(root);
	partition();
}

Intersection *SpacePartition::local_intersect(glm::vec3 a, glm::vec3 b, GeometryNode *node) {
	a = glm::vec3(node->get_squashed_invtrans() * glm::vec4(a, 1.0));
	b = glm::vec3(node->get_squashed_invtrans() * glm::vec4(b, 1.0));
	Intersection *i = node->m_primitive->intersection(a, b, NULL);
	i->node = node;
	if (i->has_intersected) {
		i->local_intersection = ray_point_at_parameter(a,b,i->t);
		i->invtrans = node->get_squashed_invtrans();
	}
	return i;
}

Intersection * SpacePartition::bb_intersect(glm::vec3 a, glm::vec3 b) {
	return NonhierBox(posn, size).intersection(a,b,NULL);
}

Intersection *SpacePartition::intersect(glm::vec3 a, glm::vec3 b) {
	Intersection *selfi = bb_intersect(a,b);
	if (!selfi->has_intersected) {
		return selfi;
	} else {
		delete selfi;
	}

	if (is_partitioned) {
		// we only want to check the closest bb hit
		// only if no intersection do we proceed to next closts bb hit


		// find closest child
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
	glm::vec3 min = get_bot_left_corner(root);
	glm::vec3 max = get_top_right_corner(root);
	posn = min;
	size = glm::max(max.x - min.x, glm::max(max.y - min.y, max.z - min.z));
	fill_root();
}
