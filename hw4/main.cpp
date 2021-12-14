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
void DrawModel(Object* model,GLuint program, GLuint vao, GLuint texture_, glm::mat4 &M, GLenum DrawingMode);
void LoadTexture(GLuint& texture, const char* tFileName, GLuint* texture_id);
void Sleep(int ms);
glm::mat4 getV();
glm::mat4 getP();
void video();
void demo();

GLuint Dissolveprogram, Modelprogram, Erect3program, Frag3program, Erect4program, Frag4program;
GLuint Umbreon_VAO, Umbreon_VBO;
GLuint Umbreon_texture_ID;
GLuint Eevee_VAO, Eevee_VBO;
GLuint Eevee_texture_ID;

GLuint UmbreonTexture;
float windowSize[2] = { 600, 600 };
float angle = 0.0f;
glm::vec3 WorldLightPos = glm::vec3(2, 5, 5);
glm::vec3 WorldCamPos = glm::vec3(7.5, 5.0, 7.5);

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

// ### hint
// comment or delete the "demo" function & feel free to chage the content of keyboard function & control parameter

//control parameter
#define Effect_TIME 240
bool rotate_f = false;
bool video_f = false;
bool model_f = true;
bool erect_f = false;
bool frag_f = false;
bool diss_f = false;
bool who = false;
unsigned int sec = 0;

void keyboard(unsigned char key, int x, int y) {
	model_f = true;
	video_f = false;
	switch (key) {
	// disable all effect
	case '1':
	{
		frag_f = false;
		erect_f = false;
		diss_f = false;
		break;
	}
	// using frag effect
	case '2':
	{
		sec = 0;
		frag_f = true;
		erect_f = false;
		diss_f = false;
		break;
	}
	// using erect effect
	case '3':
	{
		sec = 0;
		frag_f = false;
		erect_f = true;
		diss_f = false;
		break;
	}
	// using dissolve effect
	case '4':
	{
		sec = 0;
		frag_f = false;
		erect_f = false;
		diss_f = true;
		break;
	}
	case 'r':
	{
		rotate_f = !rotate_f;
		break;
	}
	case 'x':
	{
		who = !who;
		break;
	}
	case 'v':
	{
		model_f = false;
		video_f = true;
		angle = 0;
		sec = 0;
		break;
	}
	default:
	{
		break;
	}
	}
}

void shaderInit() {
	// ### hint
	// create your shader in here
	// below two shader "expand3" & "expand4" only differ in "input type of primitive" of geometry shader (one for triangle , one for quad)
	// what kind of primitive is sent to geometry shader is depend on what model you call in "glDrawArrays" function
	GLuint vert = createShader("Shaders/expand4.vert", "vertex");
	GLuint goem = createShader("Shaders/expand4.geom", "geometry");
	GLuint frag = createShader("Shaders/expand4.frag", "fragment");
	Frag4program = createProgram(vert, goem, frag);
	Erect4program = createProgram(vert, goem, frag);
	// remeber to use program before passing value to uniformal variable
	// I use a flag in geometry shader to create different effect
	glUseProgram(Erect4program);
		GLfloat vecID = glGetUniformLocation(Erect4program, "is_frag");
		glUniform1i(vecID, false);
	glUseProgram(0);

	vert = createShader("Shaders/expand3.vert", "vertex");
	goem = createShader("Shaders/expand3.geom", "geometry");
	frag = createShader("Shaders/expand3.frag", "fragment");
	Frag3program = createProgram(vert, goem, frag);
	Erect3program = createProgram(vert, goem, frag);
	glUseProgram(Erect3program);
		vecID = glGetUniformLocation(Erect3program, "is_frag");
		glUniform1i(vecID, false);
	glUseProgram(0);

	vert = createShader("Shaders/dissolve.vert", "vertex");
	frag = createShader("Shaders/dissolve.frag", "fragment");
	Dissolveprogram = createProgram(vert, 0, frag);

	vert = createShader("Shaders/model.vert", "vertex");
	frag = createShader("Shaders/model.frag", "fragment");
	Modelprogram = createProgram(vert, frag);

}

void bufferModel(Object* model, GLuint* vao, GLuint* vbo) {
	// ### hint
	// you can add what you need in "Object.h" & "Vertex.h" & buffer those data here
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

// ### hint
// load any object model you want with texture below
// be careful of some obect model may need ".mtl file" which file name shouldn't be changed and the size of model & model's default orientaion may be different.
// it's not all of the model compoesd by triangle. you can know how to draw the model by drop the object file in to visual studio & you can see what kind of polygon makes up the model
// this demo using the two kind of model. one is "Eevee" composed of triangle another one is "Umbreon" composed of quads. (by the way Pikachu is also composed of triangle)
void bindbufferInit() {
	bufferModel(Umbreon, &Umbreon_VAO, &Umbreon_VBO);
	bufferModel(Eevee, &Eevee_VAO, &Eevee_VBO);
}

void textureInit() {
	LoadTexture(UmbreonTexture, "Umbreon.jpg", &Umbreon_texture_ID);
	LoadTexture(UmbreonTexture, "Eevee.jpg", &Eevee_texture_ID);
}

void display() {
	Start = clock();
	//Clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearDepth(1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// ### hint
	// comment this function. It's just a example demo of some functionality or effect that shader can achieve.
	// you should write your own "demo" function & shader to create video.
	demo();
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

void DrawModel(Object* model, GLuint program, GLuint vao, GLuint texture_ID, glm::mat4& M, GLenum DrawingMode)
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
	glDrawArrays(DrawingMode, 0, model->positions.size() / 3);
	glBindVertexArray(0);
	glActiveTexture(0);
	glUseProgram(0);
}

void Sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
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

// ### hint
// you should change the whole content below, I think that is easier to create the effect you want than reusing the code below
void video() {
	// basic transformation matrix apply on all models
	glm::mat4 base_M(1.0f);
	base_M = glm::rotate(base_M, glm::radians(angle), glm::vec3(0, 1, 0));

	// draw Umbreon
	glm::mat4 M1(base_M);
	M1 = glm::scale(M1, glm::vec3(1, 1, 1));
	// show the model between two line(specifically is a plane)
	glm::vec4 back_line(0, 0, Umbreon->min_z, 1);
	glm::vec4 front_line(0, 0, Umbreon->max_z, 1);
	glm::vec4 base_line_normal(0, 0, 1, 0);
	// control the dissolve effect to same druation
	GLfloat Umbreon_effect_t = (Umbreon->max_z - Umbreon->min_z) / Effect_TIME;
	if (back_line.z < Umbreon->max_z) {
		back_line.z += Umbreon_effect_t * sec;
	}
	// remeber to use program before passing value to uniformal variable
	glUseProgram(Dissolveprogram);
		GLuint vecID = glGetUniformLocation(Dissolveprogram, "back_line");
		glUniform4fv(vecID, 1, &back_line[0]);
		vecID = glGetUniformLocation(Dissolveprogram, "front_line");
		glUniform4fv(vecID, 1, &front_line[0]);
		vecID = glGetUniformLocation(Dissolveprogram, "base_line_normal");
		glUniform4fv(vecID, 1, glm::value_ptr(base_line_normal));
	glUseProgram(0);
	glUseProgram(Erect4program);
		// only create the "Goosebump" effect (I think it's look like pokemon erecting its hair) in a fixed range 
		front_line = back_line + 0.5f * base_line_normal;
		vecID = glGetUniformLocation(Erect4program, "back_line");
		glUniform4fv(vecID, 1, &back_line[0]);
		vecID = glGetUniformLocation(Erect4program, "front_line");
		glUniform4fv(vecID, 1, &front_line[0]);
		vecID = glGetUniformLocation(Erect4program, "base_line_normal");
		glUniform4fv(vecID, 1, &base_line_normal[0]);
	glUseProgram(0);
	DrawModel(Umbreon, Dissolveprogram, Umbreon_VAO, Umbreon_texture_ID, M1, GL_QUADS);
	// disable detpth to prevent erect shader result to be culled
	glDepthFunc(GL_ALWAYS);
	// ### hint
	// be careful of the drawing mode of "Umbreon" is "GL_LINES_ADJACENCY" because of the Umbreon is composed of QUADS and geometry shader can't support the GL_QUADS mode
		DrawModel(Umbreon, Erect4program, Umbreon_VAO, Umbreon_texture_ID, M1, GL_LINES_ADJACENCY);
	glDepthFunc(GL_LEQUAL);

	// draw Eevee
	glm::mat4 M2(base_M);
	M2 = glm::translate(M2, glm::vec3(0, -0.8, 0));
	M2 = glm::rotate(M2, glm::radians(angle), glm::vec3(0, 1, 0));
	M2 = glm::rotate(M2, glm::radians(90.0f), glm::vec3(1, 0, 0));
	M2 = glm::scale(M2, glm::vec3(0.25, 0.25, 0.25));

	GLfloat Eevee_effect_t = (Eevee->max_y - Eevee->min_y) / Effect_TIME;
	glUseProgram(Dissolveprogram);
		// dissolve effect in inverse direction
		back_line = glm::vec4(0, Eevee->min_y, 0, 1);
		base_line_normal = glm::vec4(0, 1, 0, 0);
		front_line = back_line + Eevee_effect_t * sec;
		vecID = glGetUniformLocation(Dissolveprogram, "back_line");
		glUniform4fv(vecID, 1, &back_line[0]);
		vecID = glGetUniformLocation(Dissolveprogram, "base_line_normal");
		glUniform4fv(vecID, 1, glm::value_ptr(base_line_normal));
		vecID = glGetUniformLocation(Dissolveprogram, "front_line");
		glUniform4fv(vecID, 1, &front_line[0]);
	glUseProgram(0);
	DrawModel(Eevee, Dissolveprogram, Eevee_VAO, Eevee_texture_ID, M2, GL_TRIANGLES);
}

// ### hint
// Don't be afraid of this hw. It looks like a lot of code because I need to make sure tha all effect (3 kind of shader) can show with two model in different drawing way .
// You can just focus on one of effect(specifically reading expand.geom & knowing how it works ) to write your shader and create the effect you want to be displaied on video. 
// Reminding again : Eevee model & Umbreon model are composed of different polygon . (Don't forget Pikachu which is also provided in this homework. you can ues it.)
void demo() {
	if (model_f) {
		// basic transformation matrix apply on all models
		glm::mat4 base_M(1.0f);
		base_M = glm::rotate(base_M, glm::radians(angle), glm::vec3(0, 1, 0));
		if (who) {
			// draw Eevee
			// model matrix & transformation function to control model position
			glm::mat4 M2(base_M);
			M2 = glm::translate(M2, glm::vec3(0, -0.8, 0));
			M2 = glm::rotate(M2, glm::radians(angle), glm::vec3(0, 1, 0));
			M2 = glm::rotate(M2, glm::radians(90.0f), glm::vec3(1, 0, 0));
			M2 = glm::scale(M2, glm::vec3(0.5, 0.5, 0.5));

			// some parameter I defined to contorl the shader result 
			// be careful of the different between Eevee & Umbreon , one using the z value another one using the y value beacuse of the default orientaion of model is different
			GLfloat Eevee_effect_t = (Eevee->max_y - Eevee->min_y) / Effect_TIME;
			glm::vec4 back_line = glm::vec4(0, Eevee->min_y, 0, 1);
			glm::vec4 base_line_normal = glm::vec4(0, 1, 0, 0);
			glm::vec4 front_line = glm::vec4(0, Eevee->max_y, 0, 1);
			back_line.y += Eevee_effect_t * sec;
			if (diss_f) {
				if (back_line.y < Eevee->max_y) {
				}
				else {
					// show the whole model after effect end
					back_line = glm::vec4(0, Eevee->min_y, 0, 1);
				}
				glUseProgram(Dissolveprogram);
				GLfloat vecID = glGetUniformLocation(Dissolveprogram, "back_line");
				glUniform4fv(vecID, 1, &back_line[0]);
				vecID = glGetUniformLocation(Dissolveprogram, "base_line_normal");
				glUniform4fv(vecID, 1, glm::value_ptr(base_line_normal));
				vecID = glGetUniformLocation(Dissolveprogram, "front_line");
				glUniform4fv(vecID, 1, &front_line[0]);
				glUseProgram(0);
				DrawModel(Eevee, Dissolveprogram, Eevee_VAO, Eevee_texture_ID, M2, GL_TRIANGLES);
			}
			else if (erect_f || frag_f) {
				
				if (back_line.y < Eevee->max_y) {
					// only create the "Goosebump" effect (I think it's look like pokemon erecting its hair) in a fixed range 
					front_line = back_line + 0.5f * base_line_normal;
				}
				else {
					// show the effect of whole range
					back_line = glm::vec4(0, Eevee->min_y, 0, 1);
				}
				GLfloat program;
				if (erect_f) {
					program = Erect3program;
				}
				if (frag_f) {
					program = Frag3program;
				}
				glUseProgram(program);
				GLfloat vecID = glGetUniformLocation(program, "back_line");
				glUniform4fv(vecID, 1, &back_line[0]);
				vecID = glGetUniformLocation(program, "base_line_normal");
				glUniform4fv(vecID, 1, glm::value_ptr(base_line_normal));
				vecID = glGetUniformLocation(program, "front_line");
				glUniform4fv(vecID, 1, &front_line[0]);
				glUseProgram(0);
				DrawModel(Eevee, program, Eevee_VAO, Eevee_texture_ID, M2, GL_TRIANGLES);
			}
			else
			{
				DrawModel(Eevee, Modelprogram, Eevee_VAO, Eevee_texture_ID, M2, GL_TRIANGLES);
			}
				
		}
		else {
			// draw Umbreon
			// model matrix & transformation function to control model position
			glm::mat4 M1(base_M);
			M1 = glm::scale(M1, glm::vec3(2, 2, 2));

			// some parameter I defined to contorl the shader result
			// be careful of the different between Eevee & Umbreon , one using the z value another one using the y value beacuse the default orientaion of model is different
			glm::vec4 back_line(0, 0, Umbreon->min_z, 1);
			glm::vec4 front_line(0, 0, Umbreon->max_z, 1);
			glm::vec4 base_line_normal(0, 0, 1, 0);
			GLfloat Umbreon_effect_t = (Umbreon->max_z - Umbreon->min_z) / Effect_TIME;
			back_line.z += Umbreon_effect_t * sec;
			if (diss_f) {
				if (back_line.z < Umbreon->max_z) {
				}
				else {
					// show the whole model after effect end
					back_line = glm::vec4(0, 0, Umbreon->min_z, 1);
				}
				glUseProgram(Dissolveprogram);
				GLfloat vecID = glGetUniformLocation(Dissolveprogram, "back_line");
				glUniform4fv(vecID, 1, &back_line[0]);
				vecID = glGetUniformLocation(Dissolveprogram, "base_line_normal");
				glUniform4fv(vecID, 1, glm::value_ptr(base_line_normal));
				vecID = glGetUniformLocation(Dissolveprogram, "front_line");
				glUniform4fv(vecID, 1, &front_line[0]);
				glUseProgram(0);
				DrawModel(Umbreon, Dissolveprogram, Umbreon_VAO, Umbreon_texture_ID, M1, GL_QUADS);
			}
			else if (erect_f || frag_f) {
				if (back_line.z < Umbreon->max_z) {
					// only create the "Goosebump" effect (I think it's look like pokemon erecting its hair) in a fixed range 
					front_line = back_line + 0.5f * base_line_normal;
				}
				else {
					// show the effect of whole range
					back_line = glm::vec4(0, 0, Umbreon->min_z, 1);
				}
				GLfloat program;
				if (erect_f) {
					program = Erect4program;
				}
				if (frag_f) {
					program = Frag4program;
				}
				glUseProgram(program);
					GLfloat vecID = glGetUniformLocation(program, "back_line");
					glUniform4fv(vecID, 1, &back_line[0]);
					vecID = glGetUniformLocation(program, "front_line");
					glUniform4fv(vecID, 1, &front_line[0]);
					vecID = glGetUniformLocation(program, "base_line_normal");
					glUniform4fv(vecID, 1, &base_line_normal[0]);
				glUseProgram(0);
				// ### hint
				// be careful of the drawing mode of "Umbreon" is "GL_LINES_ADJACENCY" because the Umbreon is composed of QUADS and geometry shader can't support the GL_QUADS mode
				DrawModel(Umbreon, program, Umbreon_VAO, Umbreon_texture_ID, M1, GL_LINES_ADJACENCY);
			}
			else {
				DrawModel(Umbreon, Modelprogram, Umbreon_VAO, Umbreon_texture_ID, M1, GL_QUADS);
			}
		}
	}
	if (video_f) {
		video();
	}
	if (rotate_f) {
		angle++;
	}
}