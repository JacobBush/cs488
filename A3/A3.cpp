#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <list>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

const int POSITION_MODE = 0;
const int JOINTS_MODE = 1;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  draw_circle(false),
	  use_z_buffer(true),
	  use_frontface_culling(false),
	  use_backface_culling(false),
	  interaction_mode(0),
	  dragging(0)
{
	prev_mouse_posn = vec2(0,0);
	mouse_movement = vec2(0,0);

	resetAll();
}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.05f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		if (ImGui::BeginMenuBar())
		{
		    if (ImGui::BeginMenu("Application"))
		    {
				if( ImGui::MenuItem( "Reset Position", "I" ) ) {
					resetPosition();
				}
				if( ImGui::MenuItem( "Reset Orientation", "O" ) ) {
					resetOrientation();
				}
				if( ImGui::MenuItem( "Reset Joints", "N" ) ) {
					resetJoints();
				}
				if( ImGui::MenuItem( "Reset All", "A" ) ) {
					resetAll();
				}
				if( ImGui::MenuItem( "Quit Application", "Q" ) ) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
		        ImGui::EndMenu();
		    }
		    if (ImGui::BeginMenu("Edit"))
		    {
		        if (ImGui::MenuItem("Undo", "U")) {
		        	undo();
		        }
		        if (ImGui::MenuItem("Redo", "R")) {
		        	redo();
		        }
		        ImGui::EndMenu();
		    }
		    if (ImGui::BeginMenu("Options"))
		    {
		    	if (ImGui::MenuItem("Circle", "C", &draw_circle)) {}
		    	if (ImGui::MenuItem("Z-buffer", "Z", &use_z_buffer)) {}
		    	if (ImGui::MenuItem("Backface culling", "B", &use_backface_culling)) {}
		    	if (ImGui::MenuItem("Frontface culling", "F", &use_frontface_culling)) {}
		        ImGui::EndMenu();
		    }
		    ImGui::EndMenuBar();
		}

		ImGui::RadioButton( "Position / Orientation", &interaction_mode, POSITION_MODE);
		ImGui::RadioButton( "Joints", &interaction_mode, JOINTS_MODE);

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.ks");
		vec3 ks = node.material.ks;
		glUniform3fv(location, 1, value_ptr(ks));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.shininess");
		glUniform1f(location, node.material.shininess);
		CHECK_GL_ERRORS;

	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	if (use_z_buffer) glEnable( GL_DEPTH_TEST );
	if (use_backface_culling || use_frontface_culling) glEnable( GL_CULL_FACE );
	if (use_backface_culling && use_frontface_culling) glCullFace(GL_FRONT_AND_BACK);
	if (use_backface_culling && !use_frontface_culling) glCullFace(GL_BACK);
	if (!use_backface_culling && use_frontface_culling) glCullFace(GL_FRONT);
	
	dealWithManipulation();
	renderSceneGraph(*m_rootNode);

	if (draw_circle) {
		renderArcCircle();
	}

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
}


//
void A3::renderChildren(list<SceneNode*> children) {
	for (const SceneNode * node : children) {
		//cout << "parent of " << node->m_name << " is " << node->parent->m_name << endl;
		if (node->m_nodeType == NodeType::GeometryNode)
			renderGeometryNode((GeometryNode *) node);
		else if (node->m_nodeType == NodeType::JointNode)
			renderJointNode((JointNode *) node);
	}
}

//
void A3::renderJointNode(JointNode * node) {
	mat4 oldNodeTransform = node->get_transform();
	node->set_transform(node->parent->get_transform() * oldNodeTransform);
	renderChildren(node->children);
	node->set_transform(oldNodeTransform);
}

//
//
void A3::renderGeometryNode(GeometryNode * node) {
	mat4 oldNodeTransform = node->get_transform();
	node->set_transform(node->parent->get_transform() * oldNodeTransform);
	renderChildren(node->children);

	// If picked, render differently
	if (node->parent->m_nodeType == NodeType::JointNode) {
		JointNode *parent_joint_node = (JointNode *)(node->parent);
		if (parent_joint_node->picked) {
			cout << "parent of " << node->m_name << " (" << parent_joint_node->m_name << ") is picked." << endl;
		}
	}


	updateShaderUniforms(m_shader, *node, m_view);


	// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
	BatchInfo batchInfo = m_batchInfoMap[node->meshId];

	//-- Now render the mesh:
	m_shader.enable();
	glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
	m_shader.disable();
	node->set_transform(oldNodeTransform);
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	mat4 root_transform = root.get_transform();
	root.set_transform(puppet_translation * root_transform * puppet_rotation);

	renderChildren(root.children);

	root.set_transform(root_transform);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//
void A3::dealWithManipulation() {
	const float ROTATION_SPEED = 0.01f;
	const float TRANSLATION_SPEED = 0.0015f;
	const float TRANSLATION_DEPTH_SPEED = 0.0015f;

	if (interaction_mode == POSITION_MODE) {
		if ((dragging >> 0) & 1U) {
			// dragging by left
			puppet_translation = translate(TRANSLATION_SPEED * vec3(mouse_movement.x, -mouse_movement.y, 0)) * puppet_translation;
		}
		if ((dragging >> 1) & 1U) {
			// dragging by middle
			puppet_translation = translate(TRANSLATION_DEPTH_SPEED * vec3(0,0,mouse_movement.y)) * puppet_translation;
		}
		if ((dragging >> 2) & 1U) {
			// dragging by right
			puppet_rotation = rotate(ROTATION_SPEED * mouse_movement.x, vec3(0,1,0)) * puppet_rotation;
			puppet_rotation = rotate(ROTATION_SPEED * mouse_movement.y, vec3(1,0,0)) * puppet_rotation;
		}
		mouse_movement = vec2(0.0f,0.0f);
	} 

	if (interaction_mode == JOINTS_MODE) {
		if ((dragging >> 0) & 1U) {
			// dragging by left
		}
		if ((dragging >> 1) & 1U) {
			// dragging by middle
		}
		if ((dragging >> 2) & 1U) {
			// dragging by right
		}
		mouse_movement = vec2(0.0f,0.0f);
	}
}

//
void A3::resetPosition() {
	puppet_translation = mat4();
}

//
void A3::resetOrientation() {
	puppet_rotation = mat4();
}

//
void A3::resetJoints() {
	cout << "resetJoints" << endl;
}

//
void A3::resetAll() {
	resetPosition();
	resetOrientation();
	resetJoints();
}

//
void A3::undo() {
	cout << "undo" << endl;
}

//
void A3::redo() {
	cout << "redo" << endl;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (dragging) {
			mouse_movement.x += xPos - prev_mouse_posn.x;
			mouse_movement.y += yPos - prev_mouse_posn.y;
			prev_mouse_posn = vec2(xPos, yPos);
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// dragging is 3 bits right|middle|left
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS) {
			prev_mouse_posn = vec2(ImGui::GetMousePos().x,  ImGui::GetMousePos().y);
			dragging |= 1UL << 0;
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_RELEASE) {
			dragging &= ~(1UL << 0);
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE && actions == GLFW_PRESS) {
			prev_mouse_posn = vec2(ImGui::GetMousePos().x,  ImGui::GetMousePos().y);
			dragging |= 1UL << 1;
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE && actions == GLFW_RELEASE) {
			dragging &= ~(1UL << 1);
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && actions == GLFW_PRESS) {
			prev_mouse_posn = vec2(ImGui::GetMousePos().x,  ImGui::GetMousePos().y);
			dragging |= 1UL << 2;
			eventHandled = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && actions == GLFW_RELEASE) {
			dragging &= ~(1UL << 2);
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		} else if ( key == GLFW_KEY_Q ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		} else if ( key == GLFW_KEY_I ) {
			resetPosition();
			eventHandled = true;
		} else if ( key == GLFW_KEY_O ) {
			resetOrientation();
			eventHandled = true;
		} else if ( key == GLFW_KEY_N ) {
			resetJoints();
			eventHandled = true;
		} else if ( key == GLFW_KEY_A ) {
			resetAll();
			eventHandled = true;
		} else if ( key == GLFW_KEY_U ) {
			undo();
			eventHandled = true;
		} else if ( key == GLFW_KEY_R ) {
			redo();
			eventHandled = true;
		} else if ( key == GLFW_KEY_C ) {
			draw_circle = !draw_circle;
			eventHandled = true;
		} else if ( key == GLFW_KEY_Z ) {
			use_z_buffer = !use_z_buffer;
			eventHandled = true;
		} else if ( key == GLFW_KEY_B ) {
			use_backface_culling = !use_backface_culling;
			eventHandled = true;
		} else if ( key == GLFW_KEY_F ) {
			use_frontface_culling = !use_frontface_culling;
			eventHandled = true;
		} else if ( key == GLFW_KEY_P ) {
			interaction_mode = POSITION_MODE;
			eventHandled = true;
		} else if ( key == GLFW_KEY_J ) {
			interaction_mode = JOINTS_MODE;
			eventHandled = true;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
