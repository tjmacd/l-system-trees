/************************************************
*
*             CSCI 4110 Course Project
*
*  Draws trees using an l-system and turtle graphics
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
#include <iostream>
#include <string>

GLuint program;			// shader programs
int window;

glm::mat4 projection;	// projection matrix
float eyex, eyey, eyez;	// eye position

glm::vec3 position = glm::vec3(0.0);
double theta=1.5, phi=1.5;
double r=50.0;

std::stack<glm::mat4> matrixStack;
glm::mat4 model;
std::string lSystem;

int modelLoc;
int colourLoc;

float zmax = 0.0;
float centre = 30.0;
float angle;
char *inputFile;

struct Master {
	GLuint vao;
	int indices;
	GLuint vbuffer;
};

Master *segment;

/**
*	Defines a cylinder to be drawn multiple times
*/
Master *cylinder(double radius, double height, int sides) {
	double *x;
	double *y;
	Master *result;
	GLuint vao;
	double angle;
	double dangle;
	int i;
	GLfloat *vertices;
	GLfloat *normals;
	GLushort *indices;
	int j;
	int base;
	GLuint vbuffer;
	GLuint ibuffer;
	GLint vPosition;
	GLint vNormal;
	int nv = 3 * 2 * (sides + 1);

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

	vertices = new GLfloat[nv];
	normals = new GLfloat[nv];
	j = 0;

	/*  bottom */
	normals[j] = 0.0;
	vertices[j++] = 0.0;
	normals[j] = 0.0;
	vertices[j++] = 0.0;
	normals[j] = -1.0;
	vertices[j++] = 0.0;
	for (i = 0; i<sides; i++) {
		normals[j] = x[i];
		vertices[j++] = x[i];
		normals[j] = y[i];
		vertices[j++] = y[i];
		normals[j] = 0.0;
		vertices[j++] = 0.0;
	}

	/* top */
	normals[j] = 0.0;
	vertices[j++] = 0.0;
	normals[j] = 0.0;
	vertices[j++] = 0.0;
	normals[j] = 1.0;
	vertices[j++] = height;
	for (i = 0; i<sides; i++) {
		normals[j] = x[i];
		vertices[j++] = x[i];
		normals[j] = y[i];
		vertices[j++] = y[i];
		normals[j] = 0.0;
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
	glBufferData(GL_ARRAY_BUFFER, 2*nv * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nv * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, nv * sizeof(GLfloat), nv * sizeof(GLfloat), normals);

	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 4 * sides * sizeof(GLushort), indices, GL_STATIC_DRAW);

	vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)(nv * sizeof(GLfloat)));
	glEnableVertexAttribArray(vNormal);

	return(result);
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

	projection = glm::perspective(45.0f, ratio, 1.0f, 2*zmax);

}

/**
*	Turtle graphics functions
*/

// Moves the turtle forward
void forward(float distance) {
	model = glm::scale(model, glm::vec3(1.0, 1.0, distance));
	glUniformMatrix4fv(modelLoc, 1, 0, glm::value_ptr(model));
	glBindVertexArray(segment->vao);
	glBindBuffer(GL_ARRAY_BUFFER, segment->vbuffer);
	glDrawElements(GL_TRIANGLES, segment->indices, GL_UNSIGNED_SHORT, NULL);
	model = glm::scale(model, glm::vec3(1.0, 1.0, 1 / distance));
	model = glm::translate(model, glm::vec3(0.0, 0.0, distance));
	if (model[3].z > zmax) {
		zmax = model[3].z;
		r = zmax;
	}
}

// Turns the turtle left
void left(float angle) {
	float rangle = angle * M_PI / 180;
	model = glm::rotate(model, -rangle, glm::vec3(0.0, 1.0, 0.0));
}

// Turns the turtle right
void right(float angle) {
	float rangle = angle * M_PI / 180;
	model = glm::rotate(model, rangle, glm::vec3(0.0, 1.0, 0.0));
}

// Turns the turtle forward
void up(float angle) {
	float rangle = angle * M_PI / 180;
	model = glm::rotate(model, -rangle, glm::vec3(1.0, 0.0, 0.0));
}

// Turns the turtle backward
void down(float angle) {
	float rangle = angle * M_PI / 180;
	model = glm::rotate(model, rangle, glm::vec3(1.0, 0.0, 0.0));
}

// Pushes the current turtle state onto the stack
void push() {
	matrixStack.push(model);
}

// Pops the old turtle state off the stack
void pop() {
	model = matrixStack.top();
	matrixStack.pop();
}

// Changes the colour
void setColour(float r, float g, float b) {
	glUniform4f(colourLoc, r, g, b, 1.0);
}

// Interprets a string of instructions as turtle commands
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

/**
*	Initializes l-system and graphics variables
*/
void init() {
	segment = cylinder(0.2, 1.0, 10);
	int iDepth;
	std::string axiom;
	std::string err = loadLSystem(lSystem, angle, inputFile);
	if (!err.empty()) {
		std::cerr << err << std::endl;
		exit(1);
	}
	//printf("%s\n", lSystem.c_str());

	model = glm::mat4(1.0);

	drawLsystem(lSystem, angle, 1);
	centre = zmax / 2;

	eyex = 0.0;
	eyey = r;
	eyez = centre + 6;

}

/*
*  This procedure is called each time the screen needs
*  to be redisplayed
*/
void displayFunc() {
	glm::mat4 view;
	glm::mat4 viewPerspective;
	int viewLoc;
	int materialLoc;
	int lightLoc;
	
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
	lightLoc = glGetUniformLocation(program, "light");
	glUniform3f(lightLoc, 1.0, 1.0, 1.0);
	materialLoc = glGetUniformLocation(program, "material");
	glUniform4f(materialLoc, 0.3, 0.7, 0.7, 150.0);

	colour = glm::vec3(0.0, 1.0, 0.0);
	glUniform4f(colourLoc, 0.0, 1.0, 0.0, 1.0);

	model = glm::mat4(1.0);

	drawLsystem(lSystem, angle, 1);
	centre = zmax / 2;
	

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

	if (argc < 2) {
		printf("usage: viewer [input_file]\n");
		exit(0);
	}

	inputFile = argv[1];

	/*
	*  initialize glut, set some parameters for
	*  the application and create the window
	*/
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	window = glutCreateWindow("Course Project");

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
	vs = buildShader(GL_VERTEX_SHADER, "project.vs");
	fs = buildShader(GL_FRAGMENT_SHADER, "project.fs");
	program = buildProgram(vs, fs, 0);
	dumpProgram(program, "CSCI 4110 Course Project");

	glUseProgram(program);

	init();

	glClearColor(0.52, 0.8, 0.98, 1.0);

	glutMainLoop();

}