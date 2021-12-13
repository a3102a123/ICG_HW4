#define GLM_ENABLE_EXPERIMENTAL

#include "Object.h"
#include "FreeImage.h"
#include "glew.h"
#include "freeglut.h"
#include "shader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <string>
#include <math.h>
#include <stb_image.h>
#include <ctime>
#include <chrono>
#include <thread>
#include "Vertex.h"
#include <vector>

using namespace std;

void keyboard(unsigned char key, int x, int y);
void shaderInit();
void bufferModel(Object* model, GLuint* vao, GLuint* vbo);
void bindbufferInit();
void textureInit();
void display();
void idle();
void reshape(GLsizei w, GLsizei h);
void DrawModel(Object* model,GLuint program, GLuint vao, GLuint texture_, glm::mat4 &M, GLenum DrawingWayID);
void LoadTexture(GLuint& texture, const char* tFileName, GLuint* texture_id);
void Sleep(int ms);

GLuint Dissolveprogram, Modelprogram, Expandprogram;
GLuint Umbreon_VAO, Umbreon_VBO;
GLuint Umbreon_texture_ID;
GLuint Eevee_VAO, Eevee_VBO;
GLuint Eevee_texture_ID;

GLuint UmbreonTexture;
float windowSize[2] = { 600, 600 };
float angle = 0.0f;
glm::vec3 WorldLightPos = glm::vec3(2, 5, 5);
glm::vec3 WorldCamPos = glm::vec3(7.5, 5.0, 7.5);

bool f = false;
// feeling free to adjust below value to fit your computer efficacy.
#define MAX_FPS 120
// timer for FPS control
clock_t Start, End;

Object* Umbreon = new Object("Umbreon.obj");
Object* Eevee = new Object("Eevee.obj");

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(windowSize[0], windowSize[1]);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("hw4");

	glewInit();
	shaderInit();
	bindbufferInit();
	textureInit();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutMainLoop();
	return 0;
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'x':
	{
		f = !f;
		break;
	}
	default:
	{
		break;
	}
	}
}

void shaderInit() {
	GLuint vert = createShader("Shaders/expand.vert", "vertex");
	GLuint goem = createShader("Shaders/expand.geom", "geometry");
	GLuint frag = createShader("Shaders/expand.frag", "fragment");
	Expandprogram = createProgram(vert, goem, frag);

	vert = createShader("Shaders/dissolve.vert", "vertex");
	frag = createShader("Shaders/dissolve.frag", "fragment");
	Dissolveprogram = createProgram(vert, 0, frag);

	vert = createShader("Shaders/model.vert", "vertex");
	frag = createShader("Shaders/model.frag", "fragment");
	Modelprogram = createProgram(vert, frag);

}

void bufferModel(Object* model, GLuint* vao, GLuint* vbo) {
	vector<VertexAttribute> data;
	VertexAttribute temp;
	model->max_y = INT_MIN;
	model->min_y = INT_MAX;
	model->max_z = INT_MIN;
	model->min_z = INT_MAX;
	for (int i = 0; i < model->positions.size() / 3; i++) {
		int idx = i * 3;
		Vertex pos(model->positions[idx], model->positions[idx + 1], model->positions[idx + 2]);
		temp.setPosition(pos);
		Vertex norm(model->normals[idx], model->normals[idx + 1], model->normals[idx + 2]);
		temp.setNormal(norm);
		idx = i * 2;
		temp.setTexcoord(model->texcoords[idx], model->texcoords[idx + 1]);
		data.push_back(temp);
		model->max_y = max(model->max_y, model->positions[idx + 1]);
		model->min_y = min(model->min_y, model->positions[idx + 1]);
		model->max_z = max(model->max_z, model->positions[idx + 2]);
		model->min_z = min(model->min_z, model->positions[idx + 2]);
	}

	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);
	glBindVertexArray(*vao);

	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * data.size(), &data[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)offsetof(VertexAttribute, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)offsetof(VertexAttribute, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)offsetof(VertexAttribute, texcoord));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void bindbufferInit() {
	bufferModel(Umbreon, &Umbreon_VAO, &Umbreon_VBO);
	bufferModel(Eevee, &Eevee_VAO, &Eevee_VBO);
}

void textureInit() {
	LoadTexture(UmbreonTexture, "Umbreon.jpg", &Umbreon_texture_ID);
	LoadTexture(UmbreonTexture, "Eevee.jpg", &Eevee_texture_ID);
}

glm::mat4 getV()
{
	// set camera position and configuration
	return glm::lookAt(glm::vec3(WorldCamPos.x, WorldCamPos.y, WorldCamPos.z), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
}

glm::mat4 getP()
{
	// set perspective view
	float fov = 45.0f;
	float aspect = windowSize[0] / windowSize[1];
	float nearDistance = 1.0f;
	float farDistance = 1000.0f;
	return glm::perspective(glm::radians(fov), aspect, nearDistance, farDistance);
}

void display() {
	static unsigned int sec = 0;
	Start = clock();
	//Clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearDepth(1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// basic transformation matrix apply on all models
	glm::mat4 base_M(1.0f);
	base_M = glm::rotate(base_M, glm::radians(angle), glm::vec3(0, 1, 0));

	// draw Umbreon
	glm::mat4 M1(base_M);
	M1 = glm::scale(M1, glm::vec3(1, 1, 1));
	glm::vec4 back_line(0, 0, Umbreon->min_z,1);
	glm::vec4 front_line(0, 0, Umbreon->max_z, 1);
	glm::vec4 base_line_normal(0, 0, 1, 0);
	GLfloat range = sec * 0.01;
	if (back_line.z < Umbreon->max_z) {
			back_line.z += range;
	}
	// remeber use program before passing value to uniformal variable
	glUseProgram(Dissolveprogram);
		GLuint vecID = glGetUniformLocation(Dissolveprogram, "back_line");
		glUniform4fv(vecID, 1, &back_line[0]);
		vecID = glGetUniformLocation(Dissolveprogram, "front_line");
		glUniform4fv(vecID, 1, &front_line[0]);
		vecID = glGetUniformLocation(Dissolveprogram, "base_line_normal");
		glUniform4fv(vecID, 1, glm::value_ptr(base_line_normal));
	glUseProgram(0);
	glUseProgram(Expandprogram);
		front_line = back_line + 0.5f * base_line_normal;
		vecID = glGetUniformLocation(Expandprogram, "back_line");
		glUniform4fv(vecID, 1, &back_line[0]);
		vecID = glGetUniformLocation(Expandprogram, "front_line");
		glUniform4fv(vecID, 1, &front_line[0]);
		vecID = glGetUniformLocation(Expandprogram, "base_line_normal");
		glUniform4fv(vecID, 1, glm::value_ptr(base_line_normal));
	glUseProgram(0);
	DrawModel(Umbreon,Dissolveprogram, Umbreon_VAO, Umbreon_texture_ID, M1,GL_QUADS);
	// be careful of the drawing mode is "GL_LINES_ADJACENCY" because of the Umbreon is composed of QUADS and geometry shader can't support the GL_QUADS mode
	//DrawModel(Umbreon, Modelprogram, Umbreon_VAO, Umbreon_texture_ID, M1, GL_QUADS);
	glDepthFunc(GL_ALWAYS);
	DrawModel(Umbreon,Expandprogram, Umbreon_VAO, Umbreon_texture_ID, M1, GL_LINES_ADJACENCY);
	glDepthFunc(GL_LEQUAL);

	// draw Eevee
	glm::mat4 M2(base_M);
	M2 = glm::translate(M2, glm::vec3(0, -0.8, 0));
	M2 = glm::rotate(M2, glm::radians(angle), glm::vec3(0, 1, 0));
	M2 = glm::rotate(M2, glm::radians(90.0f), glm::vec3(1, 0, 0));
	M2 = glm::scale(M2, glm::vec3(0.25, 0.25, 0.25));

	glUseProgram(Dissolveprogram);
	back_line = glm::vec4(0, Eevee->min_y, 0, 1);
	base_line_normal = glm::vec4(0, 1, 0, 0);
	vecID = glGetUniformLocation(Dissolveprogram, "back_line");
	glUniform4fv(vecID, 1, &back_line[0]);
	vecID = glGetUniformLocation(Dissolveprogram, "base_line_normal");
	glUniform4fv(vecID, 1, glm::value_ptr(base_line_normal));
	vecID = glGetUniformLocation(Dissolveprogram, "rnage");
	glUniform1f(vecID, range);
	glUseProgram(0);
	//DrawModel(Eevee,Dissolveprogram, Eevee_VAO, Eevee_texture_ID,M2,GL_TRIANGLES);
	//DrawModel(Eevee,Modelprogram, Eevee_VAO, Eevee_texture_ID,M2,GL_TRIANGLES);
	if (f) {
		angle++;
	}
	End = clock();
	sec++;
	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h) {
	windowSize[0] = w;
	windowSize[1] = h;
}

void LoadTexture(GLuint& texture, const char* tFileName, GLuint* texture_id) {
	static GLuint idx = 0;
	*texture_id = idx;
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + idx++);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(tFileName, &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void idle() {
	// FPS control
	clock_t CostTime = End - Start;
	float PerFrameTime = 1000.0 / MAX_FPS;
	if (CostTime < PerFrameTime) {
		Sleep(ceil(PerFrameTime) - CostTime);
	}
	glutPostRedisplay();
}

void DrawModel(Object* model, GLuint program, GLuint vao, GLuint texture_ID, glm::mat4& M, GLenum DrawingWay)
{
	glUseProgram(program);

	GLuint ModelMatrixID = glGetUniformLocation(program, "M");
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &M[0][0]);

	glm::mat4 V = getV();
	ModelMatrixID = glGetUniformLocation(program, "V");
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &V[0][0]);

	glm::mat4 P = getP();
	ModelMatrixID = glGetUniformLocation(program, "P");
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &P[0][0]);

	glUniform1i(glGetUniformLocation(program, "texture"), texture_ID);

	glBindVertexArray(vao);
	glDrawArrays(DrawingWay, 0, model->positions.size() / 3);
	glBindVertexArray(0);
	glActiveTexture(0);
	glUseProgram(0);
}

void Sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
