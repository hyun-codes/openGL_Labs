#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

//
struct Ball
{
	glm::vec3 position;
	glm::vec3 velocity;
	float radius;
	glm::vec3 color;
	bool escaped = false;
};

struct Box
{
	glm::vec3 position;
	glm::vec3 velocity{0.0f};
	glm::vec3 size;
	glm::vec3 color;
	bool escaped = false;
	bool active = true;
};

enum RoomFace { Left, Right, Bottom, Top, Back };
GLuint shaderProgram = 0;
std::array<Mesh, 5> roomMeshes;
Mesh sphereMesh;
Mesh cubeMesh;
std::vector<Ball> balls;
std::array<Box, 3> boxes;
GLFWwindow* mainWindow = nullptr;
int framebufferWidth = 900;
int framebufferHeight = 800;

const glm::vec3 roomHalfSize(2.6f, 2.1f, 2.8f);
const float PI = 3.14159265359f;
const float gravityStrength = 4.0f;
const float ballSpeed = 2.5f;
float roomMoveZ = 0.0f;
float roomRotateAngleY = 0.0f;
float roomRotateAngleZ = 0.0f;
float roomRotateVelocityY = 0.0f;
bool floorOpen = false;
RoomFace openFace = Bottom;
float floorOpenAngle = 0.0f;
bool mouseDragging = false;
double lastMouseX = 0.0;

void InitRoomBuffer();
void InitSphereBuffer();
void InitCubeBuffer();
void InitBuffer();
void ResetScene();
void AddBall();
bool IsFloorSolid();
bool IsFaceSolid(int axis, int sign);
bool HasEscaped(const glm::vec3& position, const glm::vec3& halfSize);
glm::mat4 GetRoomRotation();
void ProcessInput(float deltaTime);
void UpdatePhysics(float deltaTime, const glm::vec3& gravity);
void ResolveBallBox(Ball& ball, const Box& box);
void Update(float deltaTime);
void DrawScene();
void DrawRoomScene(const glm::mat4& roomTransform);
void DrawSphereScene(const Ball& ball, const glm::mat4& roomTransform);
void DrawCubeScene(const Box& box, const glm::mat4& roomTransform);
void SetObjectTransform(const glm::mat4& transform, const glm::vec3& color);
void Cleanup();
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallBack(GLFWwindow* window, int button, int action, int mods);
void cursorCallBack(GLFWwindow* window, double x, double y);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	mainWindow = glfwCreateWindow(900, 800, "Practice21", nullptr, nullptr);
	if (!mainWindow)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(mainWindow);
	glfwSwapInterval(1);
	glfwSetKeyCallback(mainWindow, keyCallBack);
	glfwSetMouseButtonCallback(mainWindow, mouseCallBack);
	glfwSetCursorPosCallback(mainWindow, cursorCallBack);
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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (!shaderProgram)
	{
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return -1;
	}
	InitBuffer();
	ResetScene();
	std::cout << "B: add ball (max 5) | Hold z/Z: move scene along z\n"
		<< "y/Y: rotate scene clockwise/counterclockwise (press again to stop)\n"
		<< "Left mouse drag: tilt room | o: open/close floor | r: reset | q: quit" << std::endl;
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

void InitRoomBuffer()
{
	float w = roomHalfSize.x, h = roomHalfSize.y, d = roomHalfSize.z;
	// 각 평면의 정점은 방 안쪽에서 반시계 방향이다.
	const glm::vec3 corners[5][4] = {
		{{-w,-h,d}, {-w,-h,-d}, {-w,h,-d}, {-w,h,d}},
		{{w,-h,-d}, {w,-h,d}, {w,h,d}, {w,h,-d}},
		{{-w,-h,d}, {w,-h,d}, {w,-h,-d}, {-w,-h,-d}},
		{{-w,h,-d}, {w,h,-d}, {w,h,d}, {-w,h,d}},
		{{-w,-h,-d}, {w,-h,-d}, {w,h,-d}, {-w,h,-d}}
	};
	const int indices[6] = {0, 1, 2, 0, 2, 3};
	for (int face = 0; face < 5; ++face)
	{
		std::vector<Vertex> vertices;
		for (int index : indices)
		{
			glm::vec3 p = corners[face][index];
			vertices.push_back({p.x, p.y, p.z, 1, 1, 1});
		}
		InitMeshBuffer(roomMeshes[face], vertices);
	}
}

void InitSphereBuffer()
{
	std::vector<Vertex> vertices;
	const int stacks = 24, slices = 36;
	auto point = [](float latitude, float longitude) -> Vertex
	{
		glm::vec3 p(std::sin(latitude) * std::cos(longitude), std::cos(latitude), std::sin(latitude) * std::sin(longitude));
		float brightness = 0.4f + 0.6f * std::max(0.0f, glm::dot(p, glm::normalize(glm::vec3(-0.4f, 0.8f, 0.6f))));
		return {p.x, p.y, p.z, brightness, brightness, brightness};
	};
	for (int i = 0; i < stacks; ++i)
		for (int j = 0; j < slices; ++j)
		{
			float a0 = PI * i / stacks, a1 = PI * (i + 1) / stacks;
			float b0 = 2.0f * PI * j / slices, b1 = 2.0f * PI * (j + 1) / slices;
			Vertex a = point(a0, b0), b = point(a1, b0), c = point(a1, b1), d = point(a0, b1);
			// 구 표면은 바깥에서 반시계 방향으로 만든다.
			vertices.insert(vertices.end(), {a, c, b, a, d, c});
		}
	InitMeshBuffer(sphereMesh, vertices);
}

void InitCubeBuffer()
{
	const glm::vec3 positions[8] = {
		{-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f}, {0.5f,0.5f,-0.5f}, {-0.5f,0.5f,-0.5f},
		{-0.5f,-0.5f,0.5f}, {0.5f,-0.5f,0.5f}, {0.5f,0.5f,0.5f}, {-0.5f,0.5f,0.5f}
	};
	const int indices[36] = {
		0,2,1, 0,3,2, 4,5,6, 4,6,7, 4,3,0, 4,7,3,
		1,2,6, 1,6,5, 3,7,6, 3,6,2, 0,1,5, 0,5,4
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

void InitBuffer()
{
	InitRoomBuffer();
	InitSphereBuffer();
	InitCubeBuffer();
}

void AddBall()
{
	if (balls.size() >= 5) return;
	int index = static_cast<int>(balls.size());
	const glm::vec3 colors[5] = {{0.2f,0.3f,1.0f}, {0.2f,0.8f,1.0f}, {0.6f,0.3f,1.0f}, {0.1f,0.9f,0.6f}, {1.0f,0.65f,0.15f}};
	auto random = []() { return static_cast<float>(std::rand()) / RAND_MAX; };
	glm::vec3 position(-1.3f + 2.6f * random(), 0.3f + 1.0f * random(), -1.8f + 3.6f * random());
	glm::vec3 velocity((random() < 0.5f ? -1 : 1) * (1.3f + random()), 1.2f + random(), -1.5f + 3.0f * random());
	velocity = glm::normalize(velocity) * ballSpeed;
	balls.push_back({position, velocity, 0.16f + 0.015f * index, colors[index]});
}

void ResetScene()
{
	roomMoveZ = roomRotateAngleY = roomRotateAngleZ = roomRotateVelocityY = 0.0f;
	floorOpen = false;
	openFace = Bottom;
	floorOpenAngle = 0.0f;
	mouseDragging = false;
	balls.clear();
	for (int i = 0; i < 3; ++i)
	{
		float size = 0.45f + i * 0.15f;
		boxes[i] = {{0.0f, -roomHalfSize.y + size * 0.5f, -1.6f + 1.6f * i}, glm::vec3(0),
			{size, size, size}, {0.9f, 0.1f + i * 0.08f, 0.1f}};
	}
	AddBall();
}

bool IsFloorSolid()
{
	return IsFaceSolid(1, -1);
}

bool IsFaceSolid(int axis, int sign)
{
	const int axes[5] = {0, 0, 1, 1, 2};
	const int signs[5] = {-1, 1, -1, 1, -1};
	return floorOpenAngle < 3.0f || axes[openFace] != axis || signs[openFace] != sign;
}

bool HasEscaped(const glm::vec3& position, const glm::vec3& halfSize)
{
	for (int axis = 0; axis < 3; ++axis)
	{
		if (!IsFaceSolid(axis, -1) && position[axis] + halfSize[axis] < -roomHalfSize[axis]) return true;
		if (!IsFaceSolid(axis, 1) && position[axis] - halfSize[axis] > roomHalfSize[axis]) return true;
	}
	return false;
}

glm::mat4 GetRoomRotation()
{
	glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(roomRotateAngleY), glm::vec3(0, 1, 0));
	return glm::rotate(rotation, glm::radians(roomRotateAngleZ), glm::vec3(0, 0, 1));
}

void ProcessInput(float deltaTime)
{
	bool shift = glfwGetKey(mainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	if (glfwGetKey(mainWindow, GLFW_KEY_Z) == GLFW_PRESS) roomMoveZ += (shift ? -1.0f : 1.0f) * 2.0f * deltaTime;
	roomMoveZ = std::clamp(roomMoveZ, -6.0f, 3.0f);
}

void ResolveBallBox(Ball& ball, const Box& box)
{
	if (!box.active || box.escaped || ball.escaped) return;
	glm::vec3 minimum = box.position - box.size * 0.5f;
	glm::vec3 maximum = box.position + box.size * 0.5f;
	glm::vec3 nearest = glm::clamp(ball.position, minimum, maximum);
	glm::vec3 difference = ball.position - nearest;
	float distance = glm::length(difference);
	if (distance >= ball.radius) return;
	glm::vec3 normal;
	float penetration;
	if (distance > 0.00001f)
	{
		normal = difference / distance;
		penetration = ball.radius - distance;
	}
	else
	{
		float bestDistance = 100.0f;
		for (int axis = 0; axis < 3; ++axis)
			for (int sign : {-1, 1})
			{
				float toFace = sign < 0 ? ball.position[axis] - minimum[axis] : maximum[axis] - ball.position[axis];
				if (toFace < bestDistance)
				{
					bestDistance = toFace;
					normal = glm::vec3(0);
					normal[axis] = static_cast<float>(sign);
				}
			}
		penetration = ball.radius + bestDistance;
	}
	ball.position += normal * penetration;
	if (glm::dot(ball.velocity, normal) < 0.0f)
		ball.velocity = glm::reflect(ball.velocity, normal);
}

void UpdatePhysics(float deltaTime, const glm::vec3& gravity)
{
	for (Box& box : boxes)
	{
		if (!box.active) continue;
		box.velocity += gravity * deltaTime;
		box.position += box.velocity * deltaTime;
		glm::vec3 half = box.size * 0.5f;
		if (HasEscaped(box.position, half)) box.escaped = true;
		if (!box.escaped)
		{
			for (int axis = 0; axis < 3; ++axis)
			{
				float minimum = -roomHalfSize[axis] + half[axis];
				float maximum = roomHalfSize[axis] - half[axis];
				if (box.position[axis] < minimum && IsFaceSolid(axis, -1))
				{
					box.position[axis] = minimum;
					box.velocity[axis] = axis == 1 ? 0.0f : std::abs(box.velocity[axis]) * 0.15f;
				}
				if (box.position[axis] > maximum && IsFaceSolid(axis, 1))
				{
					box.position[axis] = maximum;
					box.velocity[axis] = -std::abs(box.velocity[axis]) * 0.15f;
				}
			}
			if (IsFloorSolid() && box.position.y - half.y <= -roomHalfSize.y + 0.001f)
			{
				box.velocity.x *= std::exp(-0.5f * deltaTime);
				box.velocity.z *= std::exp(-0.5f * deltaTime);
			}
		}
		if (glm::length(box.position) > 20.0f) box.active = false;
	}
	for (Ball& ball : balls)
	{
		// 공은 중력이나 감속 없이 일정한 속도로 이동한다.
		ball.position += ball.velocity * deltaTime;
		if (HasEscaped(ball.position, glm::vec3(ball.radius))) ball.escaped = true;
		if (!ball.escaped)
		{
			// 정면 평면은 그리지 않지만, 공이 내부에 머물도록 앞쪽 범위도 제한한다.
			for (int axis = 0; axis < 3; ++axis)
			{
				float minimum = -roomHalfSize[axis] + ball.radius;
				float maximum = roomHalfSize[axis] - ball.radius;
				if (ball.position[axis] < minimum && IsFaceSolid(axis, -1))
				{
					ball.position[axis] = minimum;
					ball.velocity[axis] = std::abs(ball.velocity[axis]);
				}
				if (ball.position[axis] > maximum && IsFaceSolid(axis, 1))
				{
					ball.position[axis] = maximum;
					ball.velocity[axis] = -std::abs(ball.velocity[axis]);
				}
			}
			for (const Box& box : boxes) ResolveBallBox(ball, box);
		}
	}
	balls.erase(std::remove_if(balls.begin(), balls.end(), [](const Ball& ball) { return glm::length(ball.position) > 20.0f; }), balls.end());
}

void Update(float deltaTime)
{
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	ProcessInput(deltaTime);
	roomRotateAngleY = std::fmod(roomRotateAngleY + roomRotateVelocityY * deltaTime, 360.0f);
	float target = floorOpen ? 90.0f : 0.0f;
	if (floorOpenAngle < target) floorOpenAngle = std::min(floorOpenAngle + 75.0f * deltaTime, target);
	if (floorOpenAngle > target) floorOpenAngle = std::max(floorOpenAngle - 75.0f * deltaTime, target);
	// 방이 기울어져도 중력은 화면의 아래쪽을 향한다.
	glm::vec3 gravity = glm::transpose(glm::mat3(GetRoomRotation())) * glm::vec3(0, -gravityStrength, 0);
	int steps = std::max(1, static_cast<int>(std::ceil(deltaTime * 240.0f)));
	for (int i = 0; i < steps; ++i) UpdatePhysics(deltaTime / steps, gravity);
}

void SetObjectTransform(const glm::mat4& transform, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
}

void DrawRoomScene(const glm::mat4& roomTransform)
{
	const glm::vec3 colors[5] = {{0.35f,0.42f,0.55f}, {0.55f,0.38f,0.42f},
		{0.4f,0.5f,0.35f}, {0.6f,0.55f,0.4f}, {0.5f,0.52f,0.56f}};
	for (int face = 0; face < 5; ++face)
	{
		glm::mat4 transform = roomTransform;
		if (face == openFace)
		{
			const glm::vec3 hinges[5] = {
				{-roomHalfSize.x, -roomHalfSize.y, 0}, {roomHalfSize.x, -roomHalfSize.y, 0},
				{0, -roomHalfSize.y, -roomHalfSize.z}, {0, roomHalfSize.y, -roomHalfSize.z},
				{0, -roomHalfSize.y, -roomHalfSize.z}
			};
			const glm::vec3 axes[5] = {{0,0,1}, {0,0,-1}, {1,0,0}, {-1,0,0}, {-1,0,0}};
			glm::vec3 hinge = hinges[face];
			transform = glm::translate(transform, hinge);
			transform = glm::rotate(transform, glm::radians(floorOpenAngle), axes[face]);
			transform = glm::translate(transform, -hinge);
		}
		SetObjectTransform(transform, colors[face]);
		DrawMesh(roomMeshes[face]);
	}
}

void DrawSphereScene(const Ball& ball, const glm::mat4& roomTransform)
{
	glm::mat4 transform = glm::translate(roomTransform, ball.position);
	SetObjectTransform(glm::scale(transform, glm::vec3(ball.radius)), ball.color);
	DrawMesh(sphereMesh);
}

void DrawCubeScene(const Box& box, const glm::mat4& roomTransform)
{
	if (!box.active) return;
	glm::mat4 transform = glm::translate(roomTransform, box.position);
	SetObjectTransform(glm::scale(transform, box.size), box.color);
	DrawMesh(cubeMesh);
}

void DrawScene()
{
	glClearColor(0.02f, 0.02f, 0.025f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);
	float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 9.5f), glm::vec3(0), glm::vec3(0, 1, 0));
	glm::mat4 projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 100.0f);
	SetTransformUniforms(shaderProgram, glm::mat4(1), view, projection);
	glm::mat4 roomTransform = glm::translate(glm::mat4(1), glm::vec3(0, 0, roomMoveZ)) * GetRoomRotation();
	DrawRoomScene(roomTransform);
	for (const Box& box : boxes) DrawCubeScene(box, roomTransform);
	for (const Ball& ball : balls) DrawSphereScene(ball, roomTransform);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action != GLFW_PRESS) return;
	if (key == GLFW_KEY_B) AddBall();
	if (key == GLFW_KEY_Y)
	{
		float velocity = (mods & GLFW_MOD_SHIFT) ? 35.0f : -35.0f;
		roomRotateVelocityY = roomRotateVelocityY == velocity ? 0.0f : velocity;
	}
	if (key == GLFW_KEY_O)
	{
		// 완전히 닫힌 상태에서 현재 가장 아래를 향하는 면을 선택한다.
		if (!floorOpen && floorOpenAngle == 0.0f)
		{
			const glm::vec3 normals[5] = {{-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}};
			glm::mat3 rotation(GetRoomRotation());
			float lowestY = 1.0f;
			for (int face = 0; face < 5; ++face)
			{
				float y = (rotation * normals[face]).y;
				if (y < lowestY)
				{
					lowestY = y;
					openFace = static_cast<RoomFace>(face);
				}
			}
		}
		floorOpen = !floorOpen;
	}
	if (key == GLFW_KEY_R) ResetScene();
	if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	(void)mods;
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;
	mouseDragging = action == GLFW_PRESS;
	double y;
	glfwGetCursorPos(window, &lastMouseX, &y);
}

void cursorCallBack(GLFWwindow* window, double x, double y)
{
	(void)window;
	(void)y;
	if (!mouseDragging) return;
	roomRotateAngleZ = std::fmod(roomRotateAngleZ - static_cast<float>(x - lastMouseX) * 0.35f, 360.0f);
	lastMouseX = x;
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
	for (Mesh& mesh : roomMeshes) DeleteMesh(mesh);
	DeleteMesh(sphereMesh);
	DeleteMesh(cubeMesh);
	glDeleteProgram(shaderProgram);
}
