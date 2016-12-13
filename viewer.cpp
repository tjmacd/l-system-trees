/************************************************
*
*             Lab 3
*
*  The stickman program.  This example shows
*  how a hierarchical model can be constructed
*  and displayed using OpenGL
*
************************************************/
#define _USE_MATH_DEFINES
#include <Windows.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"
#include <stdio.h>
#include <stack>
#include <math.h>
#include <map>
#include "lsystem.h"

using namespace std;

GLuint program;			// shader programs
int window;

glm::mat4 projection;	// projection matrix
float eyex, eyey, eyez;	// eye position

glm::vec3 position = glm::vec3(0.0);
double theta=1.5, phi=1.5;
double r=50.0;

stack<glm::mat4> matrixStack;
glm::mat4 model;
std::string lSystem;

int modelLoc;
int colourLoc;

float zmax = 0.0;
float centre = 30.0;
std::map<char, string> grammar;
float angle;

struct Master {
	GLuint vao;
	int indices;
	GLuint vbuffer;
};

Master *segment;

Master *cylinder(double radius, double height, int sides) {
	double *x;
	double *y;
	Master *result;
	GLuint vao;
	double angle;
	double dangle;
	int i;
	GLfloat *vertices;
	GLushort *indices;
	int j;
	int base;
	GLuint vbuffer;
	GLuint ibuffer;
	GLint vPosition;

	result = new Master;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	result->vao = vao;
	result->indices = 3 * 4 * sides;

	x = new double[sides];
	y = new double[sides];
	dangle = 6.28 / sides;
	angle = 0.0;
	for (i = 0; i<sides; i++) {
		x[i] = radius*cos(angle);
		y[i] = radius*sin(angle);
		angle += dangle;
	}

	vertices = new GLfloat[3 * 2 * (sides + 1)];
	j = 0;

	/*  bottom */
	vertices[j++] = 0.0;
	vertices[j++] = 0.0;
	vertices[j++] = 0.0;
	for (i = 0; i<sides; i++) {
		vertices[j++] = x[i];
		vertices[j++] = y[i];
		vertices[j++] = 0.0;
	}

	/* top */
	vertices[j++] = 0.0;
	vertices[j++] = 0.0;
	vertices[j++] = height;
	for (i = 0; i<sides; i++) {
		vertices[j++] = x[i];
		vertices[j++] = y[i];
		vertices[j++] = height;
	}

	indices = new GLushort[3 * 4 * sides];
	j = 0;

	/* bottom */
	for (i = 0; i<sides; i++) {
		indices[j++] = 0;
		indices[j++] = i + 1;
		indices[j++] = i + 2;
	}
	indices[j - 1] = 1;

	/* top */
	base = sides + 1;
	for (i = 0; i<sides; i++) {
		indices[j++] = base;
		indices[j++] = base + i + 1;
		indices[j++] = base + i + 2;
	}
	indices[j - 1] = base + 1;

	/* sides */
	for (i = 1; i<sides; i++) {
		indices[j++] = i;
		indices[j++] = base + i;
		indices[j++] = i + 1;
		indices[j++] = base + i;
		indices[j++] = base + i + 1;
		indices[j++] = i + 1;
	}
	indices[j++] = sides;
	indices[j++] = base + sides;
	indices[j++] = 1;
	indices[j++] = base + sides;
	indices[j++] = base + 1;
	indices[j++] = 1;

	glGenBuffers(1, &vbuffer);
	result->vbuffer = vbuffer;
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * 2 * (sides + 1) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 4 * sides * sizeof(GLushort), indices, GL_STATIC_DRAW);

	vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	return(result);
}

/*
*  The init procedure creates the OpenGL data structures
*  that contain the triangle geometry, compiles our
*  shader program and links the shader programs to
*  the data.
*/

std::string applyRules(char c) {
	if (grammar.count(c)) {
		return grammar.at(c);
	}
	else {
		return string(1,c);
	}
}

std::string processString(std::string oldString) {
	std::string newString = "";
	for (char c : oldString) {
		string a = applyRules(c);
		newString += a;
	}
	return newString;
}

std::string createLSystem(int n, std::string axiom) {
	std::string newString = axiom;
	for (int i = 0; i < n; i++) {
		newString = processString(newString);
	}
	return newString;
}



void init() {
	segment = cylinder(0.2, 1.0, 10);
	int iDepth;
	string axiom;
	std::string err = loadGrammar(iDepth, angle, axiom, grammar, "lsystem.txt");
	lSystem = createLSystem(iDepth, axiom);
	//printf("%s\n", lSystem.c_str());

}

/*
*  Executed each time the window is resized,
*  usually once at the start of the program.
*/
void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).

	if (h == 0)
		h = 1;

	float ratio = 1.0 * w / h;

	glViewport(0, 0, w, h);

	projection = glm::perspective(45.0f, ratio, 1.0f, 100.0f);

}

void forward(float distance) {
	model = glm::scale(model, glm::vec3(1.0, 1.0, distance));
	glUniformMatrix4fv(modelLoc, 1, 0, glm::value_ptr(model));
	glBindVertexArray(segment->vao);
	glBindBuffer(GL_ARRAY_BUFFER, segment->vbuffer);
	glDrawElements(GL_TRIANGLES, segment->indices, GL_UNSIGNED_SHORT, NULL);
	model = glm::scale(model, glm::vec3(1.0, 1.0, 1 / distance));
	model = glm::translate(model, glm::vec3(0.0, 0.0, distance));

}

void left(float angle) {
	float rangle = angle * M_PI / 180;
	model = glm::rotate(model, -rangle, glm::vec3(0.0, 1.0, 0.0));
}

void right(float angle) {
	float rangle = angle * M_PI / 180;
	model = glm::rotate(model, rangle, glm::vec3(0.0, 1.0, 0.0));
}

void up(float angle) {
	float rangle = angle * M_PI / 180;
	model = glm::rotate(model, rangle, glm::vec3(1.0, 0.0, 0.0));
}

void down(float angle) {
	float rangle = angle * M_PI / 180;
	model = glm::rotate(model, -rangle, glm::vec3(1.0, 0.0, 0.0));
}

void push() {
	matrixStack.push(model);
}

void pop() {
	model = matrixStack.top();
	matrixStack.pop();
}

void setColour(float r, float g, float b) {
	glUniform4f(colourLoc, r, g, b, 1.0);
}

void drawLsystem(std::string instructions, float angle, float distance) {
	for (char c : instructions) {
		switch (c) {
		case 'F':
			forward(distance);
			break;
		case '+':
			right(angle);
			break;
		case '-':
			left(angle);
			break;
		case '^':
			up(angle);
			break;
		case '&':
			down(angle);
			break;
		case '[':
			push();
			break;
		case ']':
			pop();
			break;
		case 'T':
			setColour(0.55, 0.30, 0.09);
			break;
		case 'L':
			setColour(0.13, 0.52, 0.09);
			break;
		}
	}
}

/*
*  This procedure is called each time the screen needs
*  to be redisplayed
*/
void displayFunc() {
	glm::mat4 view;
	glm::mat4 viewPerspective;
	int viewLoc;
	
	
	GLint vPosition;
	glm::vec3 colour;

	view = glm::lookAt(glm::vec3(eyex, eyey, eyez),
		glm::vec3(0.0f, 0.0f, centre),
		glm::vec3(0.0f, 0.0f, 1.0f));

	viewPerspective = projection * view;


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	viewLoc = glGetUniformLocation(program, "viewPerspective");
	glUniformMatrix4fv(viewLoc, 1, 0, glm::value_ptr(viewPerspective));
	modelLoc = glGetUniformLocation(program, "model");
	colourLoc = glGetUniformLocation(program, "colour");
	vPosition = glGetAttribLocation(program, "vPosition");

	colour = glm::vec3(0.0, 1.0, 0.0);
	glUniform4f(colourLoc, 0.0, 1.0, 0.0, 1.0);

	model = glm::mat4(1.0);

	drawLsystem(lSystem, angle, 1);
	

	/*
	forward(5);
	right(60);
	forward(5);
	left(60);
	forward(5);
	up(60);
	forward(5);
	down(60);
	forward(5);
	*/

	glutSwapBuffers();

}





/*
*  Called each time a key is pressed on
*  the keyboard.
*/
void keyboardFunc(unsigned char key, int x, int y) {

	switch (key) {
	case 'a':
		phi -= 0.1;
		break;
	case 'd':
		phi += 0.1;
		break;
	case 'w':
		theta += 0.1;
		break;
	case 's':
		theta -= 0.1;
		break;
	case 'q':
		r += 0.5;
		break;
	case 'e':
		r -= 0.5;
		break;
	}

	eyex = r*sin(theta)*cos(phi);
	eyey = r*sin(theta)*sin(phi);
	eyez = r*cos(theta)+centre;

	glutPostRedisplay();

}

int main(int argc, char **argv) {

	/*
	*  initialize glut, set some parameters for
	*  the application and create the window
	*/
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	window = glutCreateWindow("Lab Three");

	/*
	*  initialize glew
	*/
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		printf("Error starting GLEW: %s\n", glewGetErrorString(error));
		exit(0);
	}

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keyboardFunc);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	int vs;
	int fs;

	/*
	*  compile and build the shader program
	*/
	vs = buildShader(GL_VERTEX_SHADER, "example5.vs");
	fs = buildShader(GL_FRAGMENT_SHADER, "example5.fs");
	program = buildProgram(vs, fs, 0);
	dumpProgram(program, "example 5");

	glUseProgram(program);


	eyex = 0.0;
	eyey = r;
	eyez = centre+6;

	init();

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glutMainLoop();

}