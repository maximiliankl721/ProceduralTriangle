#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include "InputSystem.h"
#include "ProceduralTree.h"
#include "ShaderConstants.h"


GLuint vao;
GLuint vbo[2];

int curr_vaoi = 0;
int old_vaoi = 1;

GLint transformLoc;

InputSystem m_input;

glm::quat m_orientationQuaternion;

int tfProgram;
int renderProgram;

std::vector<float> points;

int maxIteration = 0;
int triangles = 3;

int current_id = 0;
int old_id = 1;

int firstTime = true;

void initShaders() {
	
	int vertexShaderTF = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderTF, 1,&VERTEX_SHADER_TF, NULL);
	glCompileShader(vertexShaderTF);

	int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geometryShader, 1, &GEOMETRY_SHADER, NULL);
	glCompileShader(geometryShader);

	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER, NULL);
	glCompileShader(fragmentShader);

	tfProgram = glCreateProgram();
	
	glAttachShader(tfProgram, vertexShaderTF);
	glAttachShader(tfProgram, geometryShader);
	//glAttachShader(tfProgram, fragmentShader);


	const GLchar* feedbackVaryings[] = { "outGS_position",
	"outGS_color",
	"outGS_normal",
	"outGS_length" };

	glTransformFeedbackVaryings(tfProgram, 4, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

	int vertexShaderRender = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderRender, 1, &VERTEX_SHADER_RE, NULL);
	glCompileShader(vertexShaderRender);


	renderProgram = glCreateProgram();

	glAttachShader(renderProgram, vertexShaderRender);
	glAttachShader(renderProgram, fragmentShader);

	glLinkProgram(tfProgram);
	glValidateProgram(tfProgram);
	glUseProgram(tfProgram);



	glLinkProgram(renderProgram);
	glValidateProgram(renderProgram);
	glUseProgram(renderProgram);


	transformLoc = glGetUniformLocation(renderProgram, "transformation");

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShaderTF, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShaderTF, 512, NULL, infoLog);
		std::cerr << "Vertex shader tf compilation failed: " << infoLog << std::endl;
	}
	glGetShaderiv(vertexShaderRender, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShaderRender, 512, NULL, infoLog);
		std::cerr << "Vertex shader re compilation failed: " << infoLog << std::endl;
	}
	glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
		std::cerr << "Geometry shader compilation failed: " << infoLog << std::endl;
	}
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
	}
	glGetProgramiv(tfProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(tfProgram, 512, NULL, infoLog);
		std::cerr << "Shader program tf linking failed: " << infoLog << std::endl;
	}
	glGetProgramiv(renderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(renderProgram, 512, NULL, infoLog);
		std::cerr << "Shader program render linking failed: " << infoLog << std::endl;
	}
	

}

void initBuffers() {

	

	auto vec1 = glm::vec3(0.0f,0.0f, 1.0f);
	auto vec2 = glm::vec3(0.5f, 0.5f, 0.0f);
	auto vec3 = glm::vec3(-0.5f, 0.5f, 0.0f);
	
	auto normal = normalize(cross(vec3 - vec1, vec2 - vec1));

	float length = 1.5f;

	points = {
		vec1.x, vec1.y, vec1.z, 1.0f,
		1.0f, 0.0f, 0.0f, 0.2f,
		normal.x, normal.y, normal.z, 1.0f,
		length,

		vec2.x, vec2.y, vec2.z, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		normal.x, normal.y, normal.z, 1.0f,
		length,

		vec3.x, vec3.y, vec3.z, 1.0f,
		1.0f, 1.0f, 1.0f, 0.4f,
		normal.x, normal.y, normal.z, 1.0f,
		length,
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float)  , points.data(), GL_DYNAMIC_COPY);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*) (4 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(12 * sizeof(float)));
	glEnableVertexAttribArray(3);
	
	//transform feedback buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, (points.size() * sizeof(float)* triangles), NULL, GL_DYNAMIC_COPY);

	
}

GLFWwindow* InitGLFW() {

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1024, 768, "Procedural Tree", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	return window;
}

int main() {


	auto window = InitGLFW();


	InitInputKeys(window);

	m_orientationQuaternion = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));


	initShaders();
	auto error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << error << std::endl;
	}

	initBuffers();
	error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << error << std::endl;
	}


	//TransformFeedback();
	

	double lastTime = glfwGetTime();
	double timeDifference = 0.0f;

	while (!glfwWindowShouldClose(window)) {

		UpdateInput(timeDifference);



		int screenWidth, screenHeight;
		glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
		float aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
		glViewport(0, 0, screenWidth, screenHeight);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 globalTransformation = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f) *
			glm::lookAt(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::mat4_cast(m_orientationQuaternion);

		//globalTransformation = glm::mat4(1.0f);

		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(globalTransformation));


		glUseProgram(renderProgram);
		glBindVertexArray(vao);
		//glBindBuffer(GL_ARRAY_BUFFER, vbo[current_id]);

		glDrawArrays(GL_TRIANGLES, 0, triangles);

		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);
		//glUseProgram(0);

		glfwSwapBuffers(window);

		double currentTime = glfwGetTime();
		timeDifference = currentTime - lastTime;
		lastTime = currentTime;

		
	}
}

void TransformFeedback()
{
	glUseProgram(tfProgram);
	glBindVertexArray(vao);


	for (int i = 0; i <maxIteration; ++i) {

		// transform
		glEnable(GL_RASTERIZER_DISCARD);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[current_id]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[old_id]);

		glBeginTransformFeedback(GL_TRIANGLES);

		glDrawArrays(GL_TRIANGLES, 0, triangles);

		glEndTransformFeedback();

		glDisable(GL_RASTERIZER_DISCARD);

		glFlush();

		// fill buffer and swap
		const int size = triangles * 13;
		std::vector<GLfloat> feedback;
		feedback.resize(size);
		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, feedback.size() * sizeof(GLfloat), feedback.data());
		glBufferData(GL_ARRAY_BUFFER, feedback.size() * sizeof(GLfloat), feedback.data(), GL_DYNAMIC_COPY);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, NULL);


		int t = current_id;
		current_id = old_id;
		old_id = t;
	}

	//glBindVertexArray(0);
	//glUseProgram(0);
}



void InitInputKeys(GLFWwindow* window)
{
	m_input.SetWindow(window);
	m_input.ObserveKey(GLFW_KEY_SPACE);
	m_input.ObserveKey(GLFW_KEY_RIGHT);
	m_input.ObserveKey(GLFW_KEY_LEFT);
	m_input.ObserveKey(GLFW_KEY_UP);
	m_input.ObserveKey(GLFW_KEY_DOWN);
	m_input.ObserveKey(GLFW_KEY_1);
	m_input.ObserveKey(GLFW_KEY_2);
	m_input.ObserveKey(GLFW_KEY_3);
	m_input.ObserveKey(GLFW_KEY_4);
	m_input.ObserveKey(GLFW_KEY_5);
	m_input.ObserveKey(GLFW_KEY_0);


}


void ClearBuffers()
{

	glDeleteVertexArrays(1, &vao);

	glDeleteBuffers(2, vbo);

	current_id = 0;
	old_id = 1;

}



void UpdateInput(double timeDifference)
{
	glfwPollEvents();

	m_input.Update();
	if (m_input.WasKeyPressed(GLFW_KEY_SPACE))
		m_orientationQuaternion = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));

	float xVel = 0.0f;
	if (m_input.IsKeyDown(GLFW_KEY_UP))
		xVel = glm::radians(45.0f);
	if (m_input.IsKeyDown(GLFW_KEY_DOWN))
		xVel = glm::radians(-45.0f);

	float yVel = 0.0f;
	if (m_input.IsKeyDown(GLFW_KEY_RIGHT))
		yVel = glm::radians(45.0f);
	if (m_input.IsKeyDown(GLFW_KEY_LEFT))
		yVel = glm::radians(-45.0f);

	if (m_input.WasKeyPressed(GLFW_KEY_1) && maxIteration != 1) {
		maxIteration = 1;
		triangles = 3;
		UpdateTransformFeedback();

	}
	else if (m_input.WasKeyPressed(GLFW_KEY_2) && maxIteration != 2) {
		maxIteration = 2;
		triangles = 3;
		UpdateTransformFeedback();

	}
	else if (m_input.WasKeyPressed(GLFW_KEY_3) && maxIteration != 3) {
		maxIteration = 3;
		triangles = 3;
		UpdateTransformFeedback();

	}
	else if (m_input.WasKeyPressed(GLFW_KEY_4) && maxIteration != 4) {
		maxIteration = 4;
		triangles = 3;
		UpdateTransformFeedback();

	}
	else if (m_input.WasKeyPressed(GLFW_KEY_5) && maxIteration != 5) {
		maxIteration = 7;
		triangles = 3;
		UpdateTransformFeedback();

	}
	else if (m_input.WasKeyPressed(GLFW_KEY_0) && maxIteration != 0) {
		maxIteration = 0;
		triangles = 3;
		UpdateTransformFeedback();

	}

	glm::quat velocityQuaternion = glm::quat(0.0f, glm::vec3(xVel, yVel, 0.0f));

	m_orientationQuaternion += 0.5f * ((float)timeDifference) * velocityQuaternion * m_orientationQuaternion;
	m_orientationQuaternion = normalize(m_orientationQuaternion);
}

void UpdateTransformFeedback()
{
	for (int i = 0; i <= maxIteration; ++i) {
		triangles += static_cast<int>(pow(3, i)) * 33;
	}
	ClearBuffers();
	initBuffers();
	TransformFeedback();
}
