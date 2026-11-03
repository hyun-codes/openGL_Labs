#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

struct Camera
{
	glm::vec3 target{0.0f, 0.65f, 0.0f};
	float orbitAngleY = 35.0f;
	float elevation = 35.0f;
	float distance = 11.0f;
	float fovDegrees = 45.0f;
	bool perspective = true;
};

struct Parcel
{
	int number;
	float pathOffset;
	glm::vec3 size;
	glm::vec3 color;
};

struct PathPose
{
	glm::vec3 position;
	float angleY;
};

struct CubePart
{
	glm::mat4 transform;
	glm::vec3 color;
};

GLuint shaderProgram = 0;
Mesh floorMesh;
Mesh cubeMesh;
Mesh beltMesh;
Camera camera;
std::array<Parcel, 5> parcels;
std::vector<CubePart> conveyorParts;
GLFWwindow* mainWindow = nullptr;
int framebufferWidth = 900;
int framebufferHeight = 800;

const float PI = 3.14159265359f;
const float beltHeight = 1.1f;
const float beltHalfWidth = 0.55f;
const float startX = -2.4f;
const float startZ = 2.8f;
const float firstLength = 4.1f;
const float cornerRadius = 0.8f;
const float secondLength = 4.5f;
const float beltSpeed = 0.85f;
float beltDistance = 0.0f;
bool beltMoving = true;

float GetPathLength();
PathPose GetPathPose(float distance);
glm::mat4 GetPathTransform(float distance);
void InitFloorBuffer();
void InitCubeBuffer();
void InitBeltBuffer();
void InitConveyorParts();
void InitBuffer();
void ResetScene();
void ProcessInput(float deltaTime);
void Update(float deltaTime);
void DrawScene();
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color);
void DrawPartScene(const glm::mat4& parent, const glm::vec3& center, const glm::vec3& size, const glm::vec3& color);
void DrawConveyorScene();
void DrawParcelScene(const Parcel& parcel);
void DrawNumberScene(int number, const glm::mat4& faceTransform);
void Cleanup();
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	mainWindow = glfwCreateWindow(900, 800, "Practice20", nullptr, nullptr);
	if (!mainWindow)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(mainWindow);
	glfwSwapInterval(1);
	glfwSetKeyCallback(mainWindow, keyCallBack);
	glfwSetFramebufferSizeCallback(mainWindow, framebufferCallBack);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return -1;
	}
	glfwGetFramebufferSize(mainWindow, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);
	glEnable(GL_DEPTH_TEST);
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (!shaderProgram)
	{
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return -1;
	}
	InitBuffer();
	ResetScene();
	std::cout << "b: belt start/stop | +/-: zoom | p/P: perspective/orthographic\n"
		<< "Left/Right: camera orbit | r: reset | q: quit" << std::endl;
	double lastFrameTime = glfwGetTime();
	while (!glfwWindowShouldClose(mainWindow))
	{
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - lastFrameTime);
		lastFrameTime = currentTime;
		Update(deltaTime);
		DrawScene();
		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
	}
	Cleanup();
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
	return 0;
}

float GetPathLength()
{
	return firstLength + PI * 0.5f * cornerRadius + secondLength;
}

PathPose GetPathPose(float distance)
{
	if (distance <= firstLength)
		return {{startX, beltHeight, startZ - distance}, 0.0f};
	float arcLength = PI * 0.5f * cornerRadius;
	float centerX = startX + cornerRadius;
	float centerZ = startZ - firstLength;
	if (distance < firstLength + arcLength)
	{
		float angle = (distance - firstLength) / cornerRadius;
		return {{centerX - cornerRadius * std::cos(angle), beltHeight,
			centerZ - cornerRadius * std::sin(angle)}, -glm::degrees(angle)};
	}
	return {{centerX + distance - firstLength - arcLength, beltHeight, centerZ - cornerRadius}, -90.0f};
}

glm::mat4 GetPathTransform(float distance)
{
	PathPose pose = GetPathPose(distance);
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), pose.position);
	return glm::rotate(transform, glm::radians(pose.angleY), glm::vec3(0.0f, 1.0f, 0.0f));
}

void InitFloorBuffer()
{
	std::vector<Vertex> vertices = {
		{-5.5f, 0, -5.5f, 1, 1, 1}, {5.5f, 0, -5.5f, 1, 1, 1}, {5.5f, 0, 5.5f, 1, 1, 1},
		{-5.5f, 0, -5.5f, 1, 1, 1}, {5.5f, 0, 5.5f, 1, 1, 1}, {-5.5f, 0, 5.5f, 1, 1, 1}
	};
	InitMeshBuffer(floorMesh, vertices);
}

void InitCubeBuffer()
{
	const glm::vec3 positions[8] = {
		{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
		{-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}
	};
	const int indices[36] = {
		0, 1, 2, 0, 2, 3, 5, 4, 7, 5, 7, 6, 4, 0, 3, 4, 3, 7,
		1, 5, 6, 1, 6, 2, 3, 2, 6, 3, 6, 7, 4, 5, 1, 4, 1, 0
	};
	const float brightness[6] = {0.8f, 0.65f, 0.7f, 0.9f, 1.0f, 0.55f};
	std::vector<Vertex> vertices;
	for (int i = 0; i < 36; ++i)
	{
		glm::vec3 p = positions[indices[i]];
		float color = brightness[i / 6];
		vertices.push_back({p.x, p.y, p.z, color, color, color});
	}
	InitMeshBuffer(cubeMesh, vertices);
}

void InitBeltBuffer()
{
	std::vector<Vertex> vertices;
	const int segments = 120;
	// 상자를 받칠 수 있게 시작과 끝을 조금 연장하고 벨트 상면을 삼각형으로 만든다.
	const float padding = 0.45f;
	for (int i = 0; i < segments; ++i)
	{
		float d0 = -padding + (GetPathLength() + padding * 2.0f) * i / segments;
		float d1 = -padding + (GetPathLength() + padding * 2.0f) * (i + 1) / segments;
		glm::mat4 a = GetPathTransform(d0);
		glm::mat4 b = GetPathTransform(d1);
		glm::vec3 p0 = glm::vec3(a * glm::vec4(-beltHalfWidth, 0, 0, 1));
		glm::vec3 p1 = glm::vec3(a * glm::vec4(beltHalfWidth, 0, 0, 1));
		glm::vec3 p2 = glm::vec3(b * glm::vec4(beltHalfWidth, 0, 0, 1));
		glm::vec3 p3 = glm::vec3(b * glm::vec4(-beltHalfWidth, 0, 0, 1));
		for (const glm::vec3& p : {p0, p1, p2, p0, p2, p3})
			vertices.push_back({p.x, p.y, p.z, 1, 1, 1});
	}
	InitMeshBuffer(beltMesh, vertices);
}

void InitConveyorParts()
{
	conveyorParts.clear();
	auto add = [](glm::vec3 center, glm::vec3 size, float angle, glm::vec3 color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), center);
		transform = glm::rotate(transform, angle, glm::vec3(0, 1, 0));
		conveyorParts.push_back({glm::scale(transform, size), color});
	};
	glm::vec3 railColor(0.9f, 0.75f, 0.08f);
	glm::vec3 frameColor(0.17f, 0.2f, 0.2f);
	const float padding = 0.45f;
	float cornerEnd = firstLength + PI * 0.5f * cornerRadius;
	for (float side : {-1.0f, 1.0f})
	{
		float offset = side * (beltHalfWidth + 0.035f);
		add({startX + offset, beltHeight + 0.035f, startZ - (firstLength - padding) * 0.5f},
			{0.06f, 0.10f, firstLength + padding}, 0.0f, railColor);
		PathPose end = GetPathPose(cornerEnd + secondLength * 0.5f + padding * 0.5f);
		add({end.position.x, beltHeight + 0.035f, end.position.z + offset},
			{secondLength + padding, 0.10f, 0.06f}, 0.0f, railColor);
		for (int i = 0; i < 12; ++i)
		{
			float d0 = firstLength + (cornerEnd - firstLength) * i / 12;
			float d1 = firstLength + (cornerEnd - firstLength) * (i + 1) / 12;
			glm::vec3 a = glm::vec3(GetPathTransform(d0) * glm::vec4(offset, 0.035f, 0, 1));
			glm::vec3 b = glm::vec3(GetPathTransform(d1) * glm::vec4(offset, 0.035f, 0, 1));
			glm::vec3 difference = b - a;
			float yaw = std::atan2(-difference.x, -difference.z);
			add((a + b) * 0.5f, {0.06f, 0.10f, glm::length(difference) + 0.01f}, yaw, railColor);
		}
	}
	for (float distance : {0.1f, 1.8f, 3.7f, cornerEnd + 0.4f, cornerEnd + 2.2f, GetPathLength() - 0.1f})
	{
		glm::mat4 frame = GetPathTransform(distance);
		for (float side : {-1.0f, 1.0f})
		{
			glm::vec3 position = glm::vec3(frame * glm::vec4(side * 0.47f, 0, 0, 1));
			position.y = (beltHeight - 0.05f) * 0.5f;
			add(position, {0.08f, beltHeight - 0.05f, 0.08f}, 0.0f, frameColor);
		}
		glm::mat4 beam = glm::translate(frame, glm::vec3(0, -0.07f, 0));
		conveyorParts.push_back({glm::scale(beam, glm::vec3(1.15f, 0.10f, 0.10f)), frameColor});
	}
}

void InitBuffer()
{
	InitFloorBuffer();
	InitCubeBuffer();
	InitBeltBuffer();
	InitConveyorParts();
}

void ResetScene()
{
	camera = Camera{};
	beltDistance = 0.0f;
	beltMoving = true;
	const glm::vec3 colors[5] = {
		{0.15f, 0.7f, 0.25f}, {0.15f, 0.4f, 0.9f}, {0.85f, 0.15f, 0.15f},
		{0.95f, 0.5f, 0.08f}, {0.9f, 0.8f, 0.12f}
	};
	for (int i = 0; i < 5; ++i)
		parcels[i] = {i + 1, 0.35f + GetPathLength() * i / 5.0f, {0.58f, 0.5f, 0.68f}, colors[i]};
}

void ProcessInput(float deltaTime)
{
	if (glfwGetKey(mainWindow, GLFW_KEY_LEFT) == GLFW_PRESS) camera.orbitAngleY -= 50.0f * deltaTime;
	if (glfwGetKey(mainWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.orbitAngleY += 50.0f * deltaTime;
	bool zoomIn = glfwGetKey(mainWindow, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_KP_ADD) == GLFW_PRESS;
	bool zoomOut = glfwGetKey(mainWindow, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;
	if (zoomIn) camera.distance -= 4.0f * deltaTime;
	if (zoomOut) camera.distance += 4.0f * deltaTime;
	camera.distance = std::clamp(camera.distance, 5.5f, 20.0f);
	camera.orbitAngleY = std::fmod(camera.orbitAngleY, 360.0f);
}

void Update(float deltaTime)
{
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	ProcessInput(deltaTime);
	if (beltMoving) beltDistance = std::fmod(beltDistance + beltSpeed * deltaTime, GetPathLength());
}

void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
	DrawMesh(cubeMesh);
}

void DrawPartScene(const glm::mat4& parent, const glm::vec3& center, const glm::vec3& size, const glm::vec3& color)
{
	DrawCubeScene(glm::scale(glm::translate(parent, center), size), color);
}

void DrawConveyorScene()
{
	glm::mat4 identity(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(identity));
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.42f, 0.44f, 0.43f);
	DrawMesh(beltMesh);
	for (const CubePart& part : conveyorParts) DrawCubeScene(part.transform, part.color);

	// 줄무늬와 상자는 같은 이동 거리를 사용한다.
	int count = static_cast<int>(GetPathLength() / 0.30f);
	for (int i = 0; i < count; ++i)
	{
		float distance = std::fmod(beltDistance + GetPathLength() * i / count, GetPathLength());
		DrawPartScene(GetPathTransform(distance), {0, 0.015f, 0}, {1.03f, 0.025f, 0.075f}, {0.16f, 0.18f, 0.17f});
	}
}

void DrawNumberScene(int number, const glm::mat4& faceTransform)
{
	// 각 비트는 숫자를 구성하는 일곱 막대 중 하나를 뜻한다.
	const unsigned int masks[6] = {0, 0x06, 0x5B, 0x4F, 0x66, 0x6D};
	const glm::vec3 centers[7] = {
		{0, 0.15f, 0.012f}, {0.09f, 0.075f, 0.012f}, {0.09f, -0.075f, 0.012f},
		{0, -0.15f, 0.012f}, {-0.09f, -0.075f, 0.012f}, {-0.09f, 0.075f, 0.012f}, {0, 0, 0.012f}
	};
	DrawPartScene(faceTransform, glm::vec3(0), {0.27f, 0.37f, 0.012f}, {0.95f, 0.95f, 0.85f});
	for (int i = 0; i < 7; ++i)
	{
		if ((masks[number] & (1u << i)) == 0) continue;
		bool horizontal = i == 0 || i == 3 || i == 6;
		glm::vec3 size = horizontal ? glm::vec3(0.18f, 0.025f, 0.012f) : glm::vec3(0.025f, 0.13f, 0.012f);
		DrawPartScene(faceTransform, centers[i], size, {0.04f, 0.04f, 0.04f});
	}
}

void DrawParcelScene(const Parcel& parcel)
{
	float distance = std::fmod(parcel.pathOffset + beltDistance, GetPathLength());
	glm::mat4 parcelTransform = GetPathTransform(distance);
	DrawPartScene(parcelTransform, {0, parcel.size.y * 0.5f + 0.03f, 0}, parcel.size, parcel.color);
	DrawPartScene(parcelTransform, {0.2f, parcel.size.y + 0.035f, 0}, {0.035f, 0.012f, parcel.size.z}, {0.65f, 0.5f, 0.25f});

	// 어느 방향에서 보아도 구분할 수 있도록 네 옆면과 윗면에 번호를 붙인다.
	for (int face = 0; face < 4; ++face)
	{
		float angle = 90.0f * face;
		float radians = glm::radians(angle);
		glm::vec3 position((parcel.size.x * 0.5f + 0.012f) * std::sin(radians),
			parcel.size.y * 0.5f + 0.03f, (parcel.size.z * 0.5f + 0.012f) * std::cos(radians));
		glm::mat4 faceTransform = glm::rotate(glm::translate(parcelTransform, position), radians, glm::vec3(0, 1, 0));
		DrawNumberScene(parcel.number, faceTransform);
	}
	glm::mat4 topTransform = glm::translate(parcelTransform, glm::vec3(0, parcel.size.y + 0.045f, 0));
	topTransform = glm::rotate(topTransform, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	DrawNumberScene(parcel.number, topTransform);
}

void DrawScene()
{
	glClearColor(0.5f, 0.72f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);
	float angle = glm::radians(camera.orbitAngleY);
	float elevation = glm::radians(camera.elevation);
	glm::vec3 cameraPosition = camera.target + camera.distance * glm::vec3(
		std::cos(elevation) * std::sin(angle), std::sin(elevation), std::cos(elevation) * std::cos(angle));
	glm::mat4 view = glm::lookAt(cameraPosition, camera.target, glm::vec3(0, 1, 0));
	float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
	glm::mat4 projection;
	if (camera.perspective)
		projection = glm::perspective(glm::radians(camera.fovDegrees), aspect, 0.1f, 100.0f);
	else
	{
		float halfHeight = camera.distance * std::tan(glm::radians(camera.fovDegrees) * 0.5f);
		projection = glm::ortho(-halfHeight * aspect, halfHeight * aspect, -halfHeight, halfHeight, 0.1f, 100.0f);
	}
	SetTransformUniforms(shaderProgram, glm::mat4(1.0f), view, projection);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.28f, 0.35f, 0.29f);
	DrawMesh(floorMesh);
	DrawConveyorScene();
	for (const Parcel& parcel : parcels) DrawParcelScene(parcel);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action != GLFW_PRESS) return;
	if (key == GLFW_KEY_B) beltMoving = !beltMoving;
	if (key == GLFW_KEY_P) camera.perspective = (mods & GLFW_MOD_SHIFT) == 0;
	if (key == GLFW_KEY_R) ResetScene();
	if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	framebufferWidth = std::max(width, 1);
	framebufferHeight = std::max(height, 1);
	glViewport(0, 0, width, height);
}

void Cleanup()
{
	DeleteMesh(floorMesh);
	DeleteMesh(cubeMesh);
	DeleteMesh(beltMesh);
	glDeleteProgram(shaderProgram);
}
