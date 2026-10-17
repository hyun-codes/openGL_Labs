#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

GLuint shaderProgram = 0;
Mesh floorMesh;
Mesh cubeMesh;
int framebufferWidth = 800;
int framebufferHeight = 800;

void InitFloorBuffer();
void InitCubeBuffer();
void InitBuffer();
void Update(float deltaTime);
void DrawScene();
void DrawFloorScene(const glm::mat4& view, const glm::mat4& projection);
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color,
	const glm::mat4& view, const glm::mat4& projection);
void Cleanup();
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice19", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallBack);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	glViewport(0, 0, framebufferWidth, framebufferHeight);
	glEnable(GL_DEPTH_TEST);

	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (shaderProgram == 0)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	InitBuffer();

	double lastFrameTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - lastFrameTime);
		lastFrameTime = currentTime;
		Update(deltaTime);
		DrawScene();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	Cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void InitFloorBuffer()
{
	std::vector<Vertex> vertices = {
		{-4.0f, 0.0f, -4.0f, 1.0f, 1.0f, 1.0f},
		{ 4.0f, 0.0f, -4.0f, 1.0f, 1.0f, 1.0f},
		{ 4.0f, 0.0f,  4.0f, 1.0f, 1.0f, 1.0f},
		{-4.0f, 0.0f, -4.0f, 1.0f, 1.0f, 1.0f},
		{ 4.0f, 0.0f,  4.0f, 1.0f, 1.0f, 1.0f},
		{-4.0f, 0.0f,  4.0f, 1.0f, 1.0f, 1.0f}
	};
	InitMeshBuffer(floorMesh, vertices);
}

void InitCubeBuffer()
{
	const glm::vec3 positions[8] = {
		{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
		{ 0.5f,  0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
		{-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f, 0.5f},
		{ 0.5f,  0.5f,  0.5f}, {-0.5f, 0.5f, 0.5f}
	};
	const int indices[36] = {
		0, 1, 2, 0, 2, 3,
		5, 4, 7, 5, 7, 6,
		4, 0, 3, 4, 3, 7,
		1, 5, 6, 1, 6, 2,
		3, 2, 6, 3, 6, 7,
		4, 5, 1, 4, 1, 0
	};
	std::vector<Vertex> vertices;
	const float faceBrightness[6] = {0.75f, 0.9f, 0.65f, 0.8f, 1.0f, 0.55f};
	for (int i = 0; i < 36; ++i)
	{
		const glm::vec3& p = positions[indices[i]];
		float brightness = faceBrightness[i / 6];
		vertices.push_back({p.x, p.y, p.z, brightness, brightness, brightness});
	}
	InitMeshBuffer(cubeMesh, vertices);
}

void InitBuffer()
{
	InitFloorBuffer();
	InitCubeBuffer();
}

void Update(float deltaTime)
{
	(void)deltaTime;
	// 이후 탱크 몸체와 카메라 애니메이션을 이곳에 작성한다.
}

void DrawFloorScene(const glm::mat4& view, const glm::mat4& projection)
{
	SetTransformUniforms(shaderProgram, glm::mat4(1.0f), view, projection);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.3f, 0.35f, 0.38f);
	DrawMesh(floorMesh);
}

void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color,
	const glm::mat4& view, const glm::mat4& projection)
{
	SetTransformUniforms(shaderProgram, transform, view, projection);
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
	DrawMesh(cubeMesh);
}

void DrawScene()
{
	glClearColor(0.75f, 0.85f, 0.95f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);
	float aspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
	glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 2.8f, 5.0f),
		glm::vec3(0.0f, 0.3f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

	DrawFloorScene(view, projection);
	// 탱크를 조립할 때는 이 육면체 버퍼를 각 몸체의 변환행렬과 색으로 재사용한다.
	glm::mat4 tankBase(1.0f);
	tankBase = glm::translate(tankBase, glm::vec3(0.0f, 0.25f, 0.0f));
	tankBase = glm::scale(tankBase, glm::vec3(1.2f, 0.5f, 0.8f));
	DrawCubeScene(tankBase, glm::vec3(0.32f, 0.62f, 0.36f), view, projection);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
	// 이후 방향키, 몸체 애니메이션, 카메라 명령을 이곳에 연결한다.
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	framebufferWidth = width > 0 ? width : 1;
	framebufferHeight = height > 0 ? height : 1;
	glViewport(0, 0, width, height);
}

void Cleanup()
{
	DeleteMesh(floorMesh);
	DeleteMesh(cubeMesh);
	glDeleteProgram(shaderProgram);
}
