#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
	void initGrid();
	void initCube();
	void initMarker();
	void resetColours();
	void resetGrid();

	void increaseCurrentStackSize();
	void decreaseCurrentStackSize();
	void moveCurrentColUp();
	void moveCurrentColDown();
	void moveCurrentColLeft();
	void moveCurrentColRight();
	void copyCurrentStackUp();
	void copyCurrentStackDown();
	void copyCurrentStackLeft();
	void copyCurrentStackRight();

	int getXFromInt(int n);
	int getYFromInt(int n);

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Cube geometry
	GLuint m_cube_vao; // Cube Array Object
	GLuint m_cube_vbo; // Cube Buffer Object

	// marker geometry
	GLuint m_marker_vao; // Cube Array Object
	GLuint m_marker_vbo; // Cube Buffer Object

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colour[8][3];
	int current_col;

	int *cube_counts;
	int *stack_colour;
	int active_square;

	bool dragging;
	int prev_mouse_x_posn;
	int rotation_amount;

	double zoom_amount;

	bool grad_stacks;
};
