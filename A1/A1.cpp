#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const float PI = 3.14159265f;

static const size_t DIM = 16;
static const float ROTATION_SPEED = 0.0025;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ), active_square( 0 ), grad_stacks( false ),
	prev_mouse_x_posn( 0 ), dragging( false ), rotation_amount( 0 )
{
	for (int i = 0; i < 8; i++) {
		fill(colour[i], colour[i] + 3, 0.0f);
	}

	// Point (x,y) = cube_counts[x + y*DIM]
	cube_counts = new int[DIM * DIM];
	memset(cube_counts, 0, DIM * DIM);

	stack_colour = new int[DIM * DIM];
	memset(stack_colour, -1, DIM * DIM);
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{
	delete [] cube_counts;
	delete [] stack_colour;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();
	initCube();
	initMarker();

	initColours();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

//
void A1::initCube() {

	// It would be more efficient to render in a strip,
	// But its simpler just to draw the tris individually

	size_t sz = 3*3*2*6;

	float verts[sz] = {
		// Front
		0,0,0,
		1,0,0,
		0,1,0,
		0,1,0,
		1,0,0,
		1,1,0,
		// Right
		1,0,0,
		1,0,-1,
		1,1,-1,
		1,0,0,
		1,1,-1,
		1,1,0,
		// Left
		0,0,0,
		0,1,0,
		0,0,-1,
		0,1,0,
		0,1,-1,
		0,0,-1,
		// Back
		0,0,-1,
		0,1,-1,
		1,1,-1,
		0,0,-1,
		1,1,-1,
		1,0,-1,
		//Bottom
		0,0,0,
		0,0,-1,
		1,0,-1,
		0,0,0,
		1,0,-1,
		1,0,0,
		//Top
		0,1,0,
		1,1,-1,
		0,1,-1,
		0,1,0,
		1,1,0,
		1,1,-1
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		&verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

//
void A1::initMarker() {
	size_t sz = 3*4;

	float verts[sz] = {
		0,0,0,
		1,0,0,
		0,0,-1,
		1,0,-1
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_marker_vao );
	glBindVertexArray( m_marker_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_marker_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_marker_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		&verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

void A1::initColours() {
	// Red
	colour[0][0] = 1.0;
	colour[0][1] = 0.0;
	colour[0][2] = 0.0;
	// Green
	colour[1][0] = 0.0;
	colour[1][1] = 1.0;
	colour[1][2] = 0.0;
	// Blue
	colour[2][0] = 0.0;
	colour[2][1] = 0.0;
	colour[2][2] = 1.0;
	// Yellow
	colour[3][0] = 1.0;
	colour[3][1] = 1.0;
	colour[3][2] = 0.0;
	// Magenta
	colour[4][0] = 1.0;
	colour[4][1] = 0.0;
	colour[4][2] = 1.0;
	// Cyan
	colour[5][0] = 0.0;
	colour[5][1] = 1.0;
	colour[5][2] = 1.0;
	// Orange
	colour[6][0] = 1.0;
	colour[6][1] = 0.5;
	colour[6][2] = 0.0;
	// Brown
	colour[7][0] = 0.7;
	colour[7][1] = 0.5;
	colour[7][2] = 0.3;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset Grid" ) ) {
			resetGrid();
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		for (int i = 0; i < 8; i++) {
			ImGui::PushID( i );
			ImGui::ColorEdit3( "##Colour", colour[i]);
			ImGui::SameLine();
			if( ImGui::RadioButton( "##Col", &current_col, i ) ) {
				// Select this colour.
				stack_colour[active_square] = current_col;
			}
			ImGui::PopID();
		}

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

		if( ImGui::Button( "Extra Feature: Toggle Gradient Stacks" ) ) {
			grad_stacks = !grad_stacks;
		}

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}


//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
	// Rotate based on mouse events
	static float translation_factor = float(DIM)/2.0f;
	W = glm::translate(W, vec3( translation_factor, 0, translation_factor ));
	W = glm::rotate( W, ROTATION_SPEED * rotation_amount, vec3(0,1,0) );
	W = glm::translate(W, vec3( -translation_factor, 0, -translation_factor ));

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes

		// We will by default have the cube at (0,0)
		W = glm::translate( W, vec3( 0, 0, DIM ) );
		glBindVertexArray( m_cube_vao );

		// For every value in cube_counts, draw that many cubes
		for (int i = 0; i < DIM * DIM; i++) {
			int x_coord = getXFromInt(i);
			int y_coord = -getYFromInt(i);
			int count = cube_counts[i];
			for (int j = 0; j < count; j++) {
				W = glm::translate( W, vec3( x_coord, j, y_coord ) );
				glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
				if (grad_stacks) {
					glUniform3f( col_uni, 1 - 1.0 / count * j , 0 + 1.0/count*j, 1 );
				} else {
					float *c = colour[stack_colour[i]];
					glUniform3f( col_uni, c[0], c[1], c[2]);
				}
				glDrawArrays( GL_TRIANGLES, 0, 3*2*6);
				// Undo the translate
				W = glm::translate( W, vec3( -x_coord, -j, -y_coord ) );
			}
		}

		// Highlight the active square:
		// Depth test so goes through cubes
		glDisable( GL_DEPTH_TEST );
		// Bind marker and color
		glBindVertexArray( m_marker_vao );
		glUniform3f( col_uni, 0, 0, 0 );
		// Translate and draw
		W = glm::translate( W, vec3( getXFromInt(active_square), 0, -getYFromInt(active_square) ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
		if (cube_counts[active_square] != 0) {
			// Draw on top of stack as well
			W = glm::translate( W, vec3( 0, cube_counts[active_square], 0 ) );
			glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
			glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
		}

	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		if (dragging) {
			int current_mouse_x_posn = ImGui::GetMousePos().x;
			rotation_amount += current_mouse_x_posn - prev_mouse_x_posn;
			prev_mouse_x_posn = current_mouse_x_posn;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS) {
			prev_mouse_x_posn = ImGui::GetMousePos().x;
			dragging = true;
		} else if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_RELEASE) {
			dragging = false;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		else if (key == GLFW_KEY_R) {
			resetGrid();
		}
		else if (key == GLFW_KEY_SPACE) {
			increaseCurrentStackSize();
		}
		else if (key == GLFW_KEY_BACKSPACE) {
			decreaseCurrentStackSize();
		}
		else if (key == GLFW_KEY_UP) {
			if (mods & GLFW_MOD_SHIFT) {
				copyCurrentStackUp();
			}
			moveCurrentColUp();
		}
		else if (key == GLFW_KEY_DOWN) {
			if (mods & GLFW_MOD_SHIFT) {
				copyCurrentStackDown();
			}
			moveCurrentColDown();
		}
		else if (key == GLFW_KEY_LEFT) {
			if (mods & GLFW_MOD_SHIFT) {
				copyCurrentStackLeft();
			}
			moveCurrentColLeft();
		}
		else if (key == GLFW_KEY_RIGHT) {
			if (mods & GLFW_MOD_SHIFT) {
				copyCurrentStackRight();
			}
			moveCurrentColRight();
		}
	}

	return eventHandled;
}

void A1::resetGrid() {
	cout << "resetGrid() called." << endl;
}

void A1::increaseCurrentStackSize() {
	if (cube_counts[active_square] == 0) stack_colour[active_square] = current_col;
	cube_counts[active_square]++;
}

void A1::decreaseCurrentStackSize() {
	cube_counts[active_square]--;
	if (cube_counts[active_square] < 0) cube_counts[active_square] = 0;
	if (cube_counts[active_square] == 0) stack_colour[active_square] = -1;
}

void A1::moveCurrentColUp() {
	if (getYFromInt(active_square) < DIM - 1) active_square += DIM;
}

void A1::moveCurrentColDown() {
	if (getYFromInt(active_square) > 0) active_square -= DIM;
}

void A1::moveCurrentColLeft() {
	if (getXFromInt(active_square) > 0) active_square--;
}

void A1::moveCurrentColRight() {
	if (getXFromInt(active_square) < DIM - 1) active_square++;
}

void A1::copyCurrentStackUp() {
	int new_col = active_square + DIM;
	if (getYFromInt(new_col) < DIM) {
		cube_counts[new_col] = cube_counts[active_square];
		stack_colour[new_col] = stack_colour[active_square];
	}
}

void A1::copyCurrentStackDown() {
	int new_col = active_square - DIM;
	if (getYFromInt(new_col) >= 0) {
		cube_counts[new_col] = cube_counts[active_square];
		stack_colour[new_col] = stack_colour[active_square];
	}
}

void A1::copyCurrentStackLeft() {
	int new_col = active_square - 1;
	if (getXFromInt(new_col) >= 0) {
		cube_counts[new_col] = cube_counts[active_square];
		stack_colour[new_col] = stack_colour[active_square];
	}
}

void A1::copyCurrentStackRight() {
	int new_col = active_square + 1;
	if (getXFromInt(new_col) < DIM) {
		cube_counts[new_col] = cube_counts[active_square];
		stack_colour[new_col] = stack_colour[active_square];
	}
}

int A1::getXFromInt(int n) {
	return n % DIM;
}

int A1::getYFromInt(int n) {
	return n / DIM;
}
