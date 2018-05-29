#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

const float PI = 3.14159265359f;
const float DEGREES_TO_RADIANS = PI/180;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{
	reset();
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();
}

//
void A2::reset() {
	theta = 30.0f * DEGREES_TO_RADIANS;
	aspect = 1.0f;
	near = 1.0f;
	far = 10.0f;
	interaction_mode = "Rotate Model";
	dragging = 0;
	prev_mouse_posn = vec2(0.0f,0.0f);
	mouse_movement = vec2(0.0f,0.0f);

	initPerspectiveMatrix();
	initViewMatrix();
	initModelMatrix();

	dragging_viewport_start = vec2(0,0);
	viewportPosn1 = vec2(-0.95f, -0.95f);
	viewportPosn2 = vec2(0.95f, 0.95f);
}

//
mat4 A2::translationMatrix(float dx, float dy, float dz) {
	return mat4(
		vec4(1,0,0,0),
		vec4(0,1,0,0),
		vec4(0,0,1,0),
		vec4(dx,dy,dz,1)
	);
}

mat4 A2::scaleMatrix(float sx, float sy, float sz) {
	return mat4(
		vec4(sx,0,0,0),
		vec4(0,sy,0,0),
		vec4(0,0,sz,0),
		vec4(0,0,0,1)
	);
}

// axis: 0=x, 1=y, 2=z
mat4 A2::rotationMatrix(int axis, float theta) {
	switch(axis) {
		case 0:
			return mat4(
				vec4(1,0,0,0),
				vec4(0,cos(theta), sin(theta), 0),
				vec4(0, -sin(theta), cos(theta), 0),
				vec4(0,0,0,1)
			);
		case 1:
			return mat4(
				vec4(cos(theta),0,-sin(theta),0),
				vec4(0, 1, 0, 0),
				vec4(sin(theta), 0, cos(theta), 0),
				vec4(0,0,0,1)
			);
		case 2:
			return mat4(
				vec4(cos(theta), sin(theta), 0, 0),
				vec4(-sin(theta), cos(theta), 0, 0),
				vec4(0,0,1,0),
				vec4(0,0,0,1)
			);
		default:
			throw invalid_argument( "axis must be in {0,1,2}" );
	}
}

//
void A2::initPerspectiveMatrix() {
	P = mat4(
		vec4(1/(tan(theta/2)*aspect),0,0,0),
		vec4(0, 1/tan(theta/2), 0, 0),
		vec4(0,0,-(far + near)/(far-near), -1),
		vec4(0,0,(-2*far*near)/(far-near),0)
	);
}

//
void A2::initModelMatrix() {
	M = mat4();
}

//
void A2::initViewMatrix() {
	V = translationMatrix(0.0f,0.0f,10.0f);
}

//
void A2::setPerspectiveMatrix() {
	const float FOV_SCALING_FACTOR = 0.0025f;
	const float PLANE_TRANSLATE_FACTOR = 0.0025f;
	if (interaction_mode == "Perspective") {
		if ((dragging >> 0) & 1U) {
			// dragging by left
			theta += mouse_movement.x * FOV_SCALING_FACTOR;
			if (theta < 5.0f * DEGREES_TO_RADIANS) {
				theta = 5.0f * DEGREES_TO_RADIANS;
			}
			if (theta > 160.0f * DEGREES_TO_RADIANS) {
				theta = 160.0f * DEGREES_TO_RADIANS;
			}
		}
		if ((dragging >> 1) & 1U) {
			// dragging by middle
			// From Piazza - don't need to restrict
			near += mouse_movement.x * PLANE_TRANSLATE_FACTOR;
		}
		if ((dragging >> 2) & 1U) {
			// dragging by right
			// From Piazza - don't need to restrict
			far += mouse_movement.x * PLANE_TRANSLATE_FACTOR;
		}
		mouse_movement = vec2(0.0f,0.0f);
	}
	P = mat4(
		vec4(1/(tan(theta/2)*aspect),0,0,0),
		vec4(0, 1/tan(theta/2), 0, 0),
		vec4(0,0,-(far + near)/(far-near), -1),
		vec4(0,0,(-2*far*near)/(far-near),0)
	);
}

//
void A2::setModelMatrix() {
	const float ROTATION_SPEED = 0.0025f;
	const float TRANSLATION_SPEED = 0.0025f;
	const float SCALING_SPEED = 0.0025f;
	// Will update model matrix to current
	if (interaction_mode == "Rotate Model") {
		if ((dragging >> 0) & 1U) {
			// dragging by left
			M *= rotationMatrix(0, mouse_movement.x * ROTATION_SPEED);
		}
		if ((dragging >> 1) & 1U) {
			// dragging by middle
			M *= rotationMatrix(1, mouse_movement.x * ROTATION_SPEED);
		}
		if ((dragging >> 2) & 1U) {
			// dragging by right
			M *= rotationMatrix(2, mouse_movement.x * ROTATION_SPEED);
		}
		mouse_movement = vec2(0.0f,0.0f);
	} else if (interaction_mode == "Scale Model") {
		if ((dragging >> 0) & 1U) {
			// dragging by left
			M *= scaleMatrix(1 + mouse_movement.x * SCALING_SPEED, 1, 1);
		}
		if ((dragging >> 1) & 1U) {
			// dragging by middle
			M *= scaleMatrix(1, 1 + mouse_movement.x * SCALING_SPEED, 1);
		}
		if ((dragging >> 2) & 1U) {
			// dragging by right
			M *= scaleMatrix(1, 1, 1 + mouse_movement.x * SCALING_SPEED);
		}
		mouse_movement = vec2(0.0f,0.0f);
	} else if (interaction_mode == "Translate Model") {
		if ((dragging >> 0) & 1U) {
			// dragging by left
			M *= translationMatrix(-mouse_movement.x * TRANSLATION_SPEED, 0, 0);
		}
		if ((dragging >> 1) & 1U) {
			// dragging by middle
			M *= translationMatrix(0, -mouse_movement.x * TRANSLATION_SPEED, 0);
		}
		if ((dragging >> 2) & 1U) {
			// dragging by right
			M *= translationMatrix(0, 0, -mouse_movement.x * TRANSLATION_SPEED);
		}
		mouse_movement = vec2(0.0f,0.0f);
	}
}

//
void A2::setViewMatrix() {
	const float ROTATION_SPEED = 0.0025f;
	const float TRANSLATION_SPEED = 0.0025f;

	if (interaction_mode == "Rotate View") {
		if ((dragging >> 0) & 1U) {
			// dragging by left
			V = rotationMatrix(0, mouse_movement.x * ROTATION_SPEED) * V;
		}
		if ((dragging >> 1) & 1U) {
			// dragging by middle
			V = rotationMatrix(1, mouse_movement.x * ROTATION_SPEED) * V;
		}
		if ((dragging >> 2) & 1U) {
			// dragging by right
			V = rotationMatrix(2, mouse_movement.x * ROTATION_SPEED) * V;
		}
		mouse_movement = vec2(0.0f,0.0f);
	} if (interaction_mode == "Translate View") {
		if ((dragging >> 0) & 1U) {
			// dragging by left
			V *= translationMatrix(mouse_movement.x * TRANSLATION_SPEED, 0, 0);
		}
		if ((dragging >> 1) & 1U) {
			// dragging by middle
			V *= translationMatrix(0, mouse_movement.x * TRANSLATION_SPEED, 0);
		}
		if ((dragging >> 2) & 1U) {
			// dragging by right
			V *= translationMatrix(0, 0, mouse_movement.x * TRANSLATION_SPEED);
		}
		mouse_movement = vec2(0.0f,0.0f);
	}
}

//
void A2::setViewport() {
	if (interaction_mode == "Viewport") {
		if ((dragging >> 0) & 1U) {
			// dragging by left
			dragging_viewport_end += mouse_movement;
			int screen_width, screen_height;
			glfwGetWindowSize(m_window, &screen_width, &screen_height);
			viewportPosn1 = vec2(
				(2.0f * dragging_viewport_start.x/screen_width) - 1.0f, 
				(2.0f * (1.0f - dragging_viewport_start.y/screen_height)) - 1.0f);
			viewportPosn2 = vec2(
				(2.0f * dragging_viewport_end.x/screen_width) - 1.0f, 
				(2.0f * (1.0f - dragging_viewport_end.y/screen_height)) - 1.0f);
		}
		mouse_movement = vec2(0.0f,0.0f);
	}
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

vec2 vec4to2(vec4 vec) {
	return vec2(vec[0], vec[1]);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	vec4 cube[8] = {
		vec4(1.0f,1.0f,1.0f,1.0f),
		vec4(1.0f,1.0f,-1.0f,1.0f),
		vec4(1.0f,-1.0f,1.0f,1.0f),
		vec4(-1.0f,1.0f,1.0f,1.0f),
		vec4(1.0f,-1.0f,-1.0f,1.0f),
		vec4(-1.0f,1.0f,-1.0f,1.0f),
		vec4(-1.0f,-1.0f,1.0f,1.0f),
		vec4(-1.0f,-1.0f,-1.0f,1.0f)
	};

	vec4 model_gnomon[4] = {
		vec4(0.0f,0.0f,0.0f,1.0f),
		vec4(-1.0f, 0.0f, 0.0f, 1.0f),
		vec4(0.0f, -1.0f, 0.0f, 1.0f),
		vec4(0.0f, 0.0f, -1.0f, 1.0f)
	};

	vec4 world_gnomon[4] = {
		vec4(0.0f,0.0f,0.0f,1.0f),
		vec4(-1.0f, 0.0f, 0.0f, 1.0f),
		vec4(0.0f, -1.0f, 0.0f, 1.0f),
		vec4(0.0f, 0.0f, -1.0f, 1.0f)
	};

	setModelMatrix();
	setViewMatrix();
	setPerspectiveMatrix();
	setViewport();

	// transform cube
	for (int i = 0; i < 8; i++) {
		cube[i] = P * V * M * cube[i];
		cube[i] /= cube[i][3];
	}

	// transform model gnomon
	for (int i = 0; i < 4; i++) {
		model_gnomon[i] = P * V * M * model_gnomon[i];
		model_gnomon[i] /= model_gnomon[i][3];

		world_gnomon[i] = P * V * world_gnomon[i];
		world_gnomon[i] /= world_gnomon[i][3];
	}

//	cout << "P = " << P << endl;
//	cout << "V = " << V << endl;
//	cout << "M = " << M << endl;
/*
	for (int i = 0; i < 8; i++) {
		cout << "cube vertex: " << vec4to2(cube[i]) << endl;
	}*/

	// draw model gnomon

	setLineColour(vec3(1.0f, 0.0f, 0.0f));
	drawLine(vec4to2(model_gnomon[0]), vec4to2(model_gnomon[1]));
	setLineColour(vec3(0.0f, 1.0f, 0.0f));
	drawLine(vec4to2(model_gnomon[0]), vec4to2(model_gnomon[2]));
	setLineColour(vec3(0.0f, 0.0f, 1.0f));
	drawLine(vec4to2(model_gnomon[0]), vec4to2(model_gnomon[3]));

	// draw model gnomon

	setLineColour(vec3(1.0f, 1.0f, 0.0f));
	drawLine(vec4to2(world_gnomon[0]), vec4to2(world_gnomon[1]));
	setLineColour(vec3(0.0f, 1.0f, 1.0f));
	drawLine(vec4to2(world_gnomon[0]), vec4to2(world_gnomon[2]));
	setLineColour(vec3(1.0f, 0.0f, 1.0f));
	drawLine(vec4to2(world_gnomon[0]), vec4to2(world_gnomon[3]));

	// draw cube
	setLineColour(vec3(0.0f,0.0f,0.0f));
	drawLine(vec4to2(cube[0]), vec4to2(cube[1]));
	drawLine(vec4to2(cube[0]), vec4to2(cube[2]));
	drawLine(vec4to2(cube[0]), vec4to2(cube[3]));
	drawLine(vec4to2(cube[5]), vec4to2(cube[1]));
	drawLine(vec4to2(cube[5]), vec4to2(cube[3]));
	drawLine(vec4to2(cube[2]), vec4to2(cube[4]));
	drawLine(vec4to2(cube[2]), vec4to2(cube[6]));
	drawLine(vec4to2(cube[1]), vec4to2(cube[4]));
	drawLine(vec4to2(cube[6]), vec4to2(cube[3]));
	drawLine(vec4to2(cube[4]), vec4to2(cube[7]));
	drawLine(vec4to2(cube[5]), vec4to2(cube[7]));
	drawLine(vec4to2(cube[6]), vec4to2(cube[7]));

	// Draw Viewport
	
	setLineColour(vec3(1.0f, 1.0f, 1.0f));

	vec2 p3 = vec2(viewportPosn1.x, viewportPosn2.y);
	vec2 p4 = vec2(viewportPosn2.x, viewportPosn1.y);
	drawLine(viewportPosn1, p3);
	drawLine(viewportPosn1, p4);
	drawLine(viewportPosn2, p3);
	drawLine(viewportPosn2, p4);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if( ImGui::Button( "Reset Application" ) ) {
			reset();
		}
		
		ImGui::Separator();

		if( ImGui::Button( "Rotate View Mode" ) ) {
			interaction_mode = "Rotate View";
		}
		if( ImGui::Button( "Translate View Mode" ) ) {
			interaction_mode = "Translate View";
		}
		if( ImGui::Button( "Perspective Mode" ) ) {
			interaction_mode = "Perspective";
		}
		if( ImGui::Button( "Rotate Model Mode" ) ) {
			interaction_mode = "Rotate Model";
		}
		if( ImGui::Button( "Translate Model Mode" ) ) {
			interaction_mode = "Translate Model";
		}
		if( ImGui::Button( "Scale Model Mode" ) ) {
			interaction_mode = "Scale Model";
		}
		if( ImGui::Button( "Draw Viewport Mode" ) ) {
			interaction_mode = "Viewport";
		}

		ImGui::Separator();

		ImGui::Text( "Interaction Mode: %s", interaction_mode.c_str() );
		ImGui::Text( "Near Plane Distance: %.1f", near );
		ImGui::Text( "Far Plane Distance: %.1f", far );
		ImGui::Text( "Field of View: %.1f degrees", (1/DEGREES_TO_RADIANS) * theta );
		ImGui::Text( "Aspect Ratio: %.1f", aspect );
		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
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
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// dragging is 3 bits right|middle|left

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS) {
			prev_mouse_posn = vec2(ImGui::GetMousePos().x,  ImGui::GetMousePos().y);
			dragging |= 1UL << 0;
			if (interaction_mode == "Viewport") {
				dragging_viewport_start = prev_mouse_posn;
				dragging_viewport_end = dragging_viewport_start;
			}
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
bool A2::mouseScrollEvent (
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
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		} else if (key == GLFW_KEY_A) {
			reset();
			eventHandled = true;
		} else if (key == GLFW_KEY_R) {
			interaction_mode = "Rotate Model";
			eventHandled = true;
		} else if (key == GLFW_KEY_T) {
			interaction_mode = "Translate Model";
			eventHandled = true;
		} else if (key == GLFW_KEY_S) {
			interaction_mode = "Scale Model";
			eventHandled = true;
		} else if (key == GLFW_KEY_P) {
			interaction_mode = "Perspective";
			eventHandled = true;
		} else if (key == GLFW_KEY_N) {
			interaction_mode = "Translate View";
			eventHandled = true;
		} else if (key == GLFW_KEY_O) {
			interaction_mode = "Rotate View";
			eventHandled = true;
		} else if (key == GLFW_KEY_V) {
			interaction_mode = "Viewport";
			eventHandled = true;
		}
	}

	return eventHandled;
}
