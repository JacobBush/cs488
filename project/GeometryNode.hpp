#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"
#include "TextureMap.hpp"
#include "BumpMap.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	void setMaterial( Material *material );
	void setTextureMap( TextureMap *texture_map );
	void setBumpMap( BumpMap *bump_map );

	// bounding volume
	glm::vec3 get_bb_bottom_left_corner();
	glm::vec3 get_bb_top_right_corner();

	Material *m_material;
	Primitive *m_primitive;

	TextureMap *m_texture_map;
	BumpMap *m_bump_map;
};
