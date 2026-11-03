#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <ctime>

// 로봇의 위치는 두 발 아래의 바닥 중심이다.
struct Robot
{
	glm::vec3 position{-3.0f, 0.0f, 2.5f};
	glm::vec3 direction{0.0f, 0.0f, -1.0f};
	float rotateAngleY = 0.0f;
	float walkSpeed = 1.5f;
	float walkPhase = 0.0f;
	float limbSwingAngle = 0.0f;
	float jumpVelocity = 0.0f;
	bool isGrounded = true;
};

struct Obstacle
{
	glm::vec3 position;
	glm::vec3 size;
	glm::vec3 color;
};

struct Camera
{
	glm::vec3 position{0.0f, 4.8f, 13.5f};
	glm::vec3 target{0.0f, 2.3f, 0.0f};
	float orbitVelocityY = 0.0f;
};

enum StageFace { Left, Right, Bottom, Top, Back, Front };
GLFWwindow* mainWindow = nullptr;
GLuint shaderProgram = 0;
Mesh cubeMesh;
std::array<Mesh, 6> stageMeshes;
Robot robot;
Camera camera;
std::array<Obstacle, 3> obstacles;
int framebufferWidth = 1000;
int framebufferHeight = 800;

const float stageHalfWidth = 4.5f;
const float stageHalfDepth = 4.5f;
const float stageHeight = 5.5f;
const float robotHalfWidth = 0.55f;
const float robotHalfDepth = 0.55f;
const float robotHeight = 2.1f;
const float jumpSpeed = 6.5f;
const float gravity = 10.0f;
bool isFrontOpen = false;
float frontOpenAmount = 0.0f;

void InitBuffer();
void InitCubeBuffer();
void InitStageBuffer();
void InitObstacles();
void ResetScene();
bool OverlapsObstacle(const glm::vec3& position, const Obstacle& obstacle);
float GetSupportHeight(const glm::vec3& position);
bool CanMoveTo(const glm::vec3& position);
void SetRobotDirection(const glm::vec3& direction);
void StartJump();
void ProcessCameraInput(float deltaTime);
void UpdateStage(float deltaTime);
void UpdateRobotJump(float deltaTime);
void UpdateRobotMovement(float deltaTime);
void UpdateRobotWalking(float deltaTime, bool moved);
void UpdateCamera(float deltaTime);
void Update(float deltaTime);
void DrawScene();
void DrawStageScene();
void DrawObstacleScene();
void DrawRobotScene();
void DrawRobotLimb(const glm::mat4& robotTransform, const glm::vec3& joint,
	float angle, float length, float width, const glm::vec3& color);
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color);
void DrawPartScene(const glm::mat4& parent, const glm::vec3& center,
	const glm::vec3& size, const glm::vec3& color);
void Cleanup();
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	mainWindow = glfwCreateWindow(1000, 800, "Practice23", nullptr, nullptr);
	if (!mainWindow) { glfwTerminate(); return -1; }
	glfwMakeContextCurrent(mainWindow);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { glfwDestroyWindow(mainWindow); glfwTerminate(); return -1; }
	glfwSwapInterval(1);
	glfwSetKeyCallback(mainWindow, keyCallBack);
	glfwSetFramebufferSizeCallback(mainWindow, framebufferCallBack);
	glfwGetFramebufferSize(mainWindow, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (!shaderProgram) { Cleanup(); glfwDestroyWindow(mainWindow); glfwTerminate(); return -1; }
	InitBuffer();
	ResetScene();
	std::cout << "W/A/S/D: walk direction | +/-: walk speed | J: jump\n"
		<< "O: front open/close | I: reset | Q: quit\n"
		<< "Hold z/Z: camera forward/back | x/X: camera left/right\n"
		<< "y/Y: camera orbit in opposite directions (same key: stop)\n";
	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(mainWindow))
	{
		double currentTime = glfwGetTime();
		Update(static_cast<float>(currentTime - lastTime));
		lastTime = currentTime;
		DrawScene();
		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
	}
	Cleanup();
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
	return 0;
}

// 초기화: 정점은 한 번만 버퍼에 저장한다.
void InitBuffer()
{
	InitCubeBuffer();
	InitStageBuffer();
}

void InitCubeBuffer()
{
	const glm::vec3 points[8] = {
		{-.5f,-.5f,-.5f}, {.5f,-.5f,-.5f}, {.5f,.5f,-.5f}, {-.5f,.5f,-.5f},
		{-.5f,-.5f,.5f}, {.5f,-.5f,.5f}, {.5f,.5f,.5f}, {-.5f,.5f,.5f}
	};
	const int indices[36] = {
		0,2,1, 0,3,2, 5,7,4, 5,6,7, 4,3,0, 4,7,3,
		1,6,5, 1,2,6, 3,6,2, 3,7,6, 4,1,5, 4,0,1
	};
	const float brightness[6] = {.85f,.75f,.7f,.9f,1.0f,.6f};
	std::vector<Vertex> vertices;
	for (int i = 0; i < 36; ++i)
	{
		glm::vec3 p = points[indices[i]];
		float c = brightness[i / 6];
		vertices.push_back({p.x, p.y, p.z, c, c, c});
	}
	InitMeshBuffer(cubeMesh, vertices);
}

void InitStageBuffer()
{
	float w = stageHalfWidth, d = stageHalfDepth, h = stageHeight;
	// 안쪽이 정면이므로 가까운 벽의 바깥쪽은 컬링되어 내부가 보인다.
	const glm::vec3 corners[6][4] = {
		{{-w,0,d}, {-w,0,-d}, {-w,h,-d}, {-w,h,d}},
		{{w,0,-d}, {w,0,d}, {w,h,d}, {w,h,-d}},
		{{-w,0,d}, {w,0,d}, {w,0,-d}, {-w,0,-d}},
		{{-w,h,-d}, {w,h,-d}, {w,h,d}, {-w,h,d}},
		{{-w,0,-d}, {w,0,-d}, {w,h,-d}, {-w,h,-d}},
		{{w,0,d}, {-w,0,d}, {-w,h,d}, {w,h,d}}
	};
	const int indices[6] = {0,1,2,0,2,3};
	for (int face = 0; face < 6; ++face)
	{
		std::vector<Vertex> vertices;
		for (int index : indices)
		{
			glm::vec3 p = corners[face][index];
			vertices.push_back({p.x,p.y,p.z,1,1,1});
		}
		InitMeshBuffer(stageMeshes[face], vertices);
	}
}

void InitObstacles()
{
	auto random = [](float minimum, float maximum) {
		return minimum + (maximum - minimum) * static_cast<float>(std::rand()) / RAND_MAX;
	};
	const glm::vec3 colors[3] = {{.95f,.5f,.1f}, {.6f,.3f,.8f}, {.55f,.65f,.7f}};
	for (int i = 0; i < 3; ++i)
	{
		Obstacle candidate;
		bool valid;
		do
		{
			candidate.size = {random(.7f,1.4f),random(.45f,1.25f),random(.7f,1.4f)};
			candidate.position = {random(-3.3f,3.3f),candidate.size.y * .5f,random(-3.3f,3.3f)};
			candidate.color = colors[i];
			valid = !OverlapsObstacle(robot.position, candidate);
			for (int j = 0; j < i; ++j)
			{
				glm::vec3 gap = glm::abs(candidate.position - obstacles[j].position);
				glm::vec3 limit = (candidate.size + obstacles[j].size) * .5f + glm::vec3(.8f);
				if (gap.x < limit.x && gap.z < limit.z) valid = false;
			}
		} while (!valid);
		obstacles[i] = candidate;
	}
}

void ResetScene()
{
	robot = Robot{};
	camera = Camera{};
	isFrontOpen = false;
	frontOpenAmount = 0.0f;
	InitObstacles();
}

// 충돌: 로봇의 바닥 면적과 장애물의 바닥 면적을 비교한다.
bool OverlapsObstacle(const glm::vec3& position, const Obstacle& obstacle)
{
	glm::vec3 half = obstacle.size * .5f;
	return std::abs(position.x - obstacle.position.x) < half.x + robotHalfWidth
		&& std::abs(position.z - obstacle.position.z) < half.z + robotHalfDepth;
}

float GetSupportHeight(const glm::vec3& position)
{
	float height = 0.0f;
	for (const Obstacle& obstacle : obstacles)
	{
		float top = obstacle.position.y + obstacle.size.y * .5f;
		if (OverlapsObstacle(position, obstacle) && position.y >= top - .001f)
			height = std::max(height, top);
	}
	return height;
}

bool CanMoveTo(const glm::vec3& position)
{
	if (std::abs(position.x) > stageHalfWidth - robotHalfWidth
		|| std::abs(position.z) > stageHalfDepth - robotHalfDepth) return false;
	for (const Obstacle& obstacle : obstacles)
	{
		float top = obstacle.position.y + obstacle.size.y * .5f;
		float bottom = obstacle.position.y - obstacle.size.y * .5f;
		if (OverlapsObstacle(position, obstacle) && position.y < top - .001f
			&& position.y + robotHeight > bottom) return false;
	}
	return true;
}

void SetRobotDirection(const glm::vec3& direction)
{
	robot.direction = direction;
	robot.rotateAngleY = glm::degrees(std::atan2(-direction.x, -direction.z));
}

void StartJump()
{
	if (!robot.isGrounded) return;
	robot.isGrounded = false;
	robot.jumpVelocity = jumpSpeed;
}

// 상태 갱신: 문, 점프, 이동, 팔다리, 카메라를 각각 처리한다.
void UpdateStage(float deltaTime)
{
	float target = isFrontOpen ? 1.0f : 0.0f;
	if (frontOpenAmount < target) frontOpenAmount = std::min(target, frontOpenAmount + deltaTime);
	if (frontOpenAmount > target) frontOpenAmount = std::max(target, frontOpenAmount - deltaTime);
}

void UpdateRobotJump(float deltaTime)
{
	float support = GetSupportHeight(robot.position);
	if (robot.isGrounded && robot.position.y > support + .001f) robot.isGrounded = false;
	if (robot.isGrounded) return;
	float previousY = robot.position.y;
	robot.jumpVelocity -= gravity * deltaTime;
	robot.position.y += robot.jumpVelocity * deltaTime;
	// 내려올 때 지나간 장애물 윗면 중 가장 높은 곳에 착지한다.
	if (robot.jumpVelocity <= 0.0f)
	{
		float landingHeight = 0.0f;
		for (const Obstacle& obstacle : obstacles)
		{
			float top = obstacle.position.y + obstacle.size.y * .5f;
			if (OverlapsObstacle(robot.position, obstacle) && previousY >= top - .001f)
				landingHeight = std::max(landingHeight, top);
		}
		if (robot.position.y <= landingHeight)
		{
			robot.position.y = landingHeight;
			robot.jumpVelocity = 0.0f;
			robot.isGrounded = true;
		}
	}
}

void UpdateRobotWalking(float deltaTime, bool moved)
{
	if (!moved) return;
	robot.walkPhase = std::fmod(robot.walkPhase + robot.walkSpeed * 220.0f * deltaTime, 360.0f);
	float amplitude = 12.0f + robot.walkSpeed * 9.0f;
	robot.limbSwingAngle = amplitude * std::sin(glm::radians(robot.walkPhase));
}

void UpdateRobotMovement(float deltaTime)
{
	glm::vec3 nextPosition = robot.position + robot.direction * robot.walkSpeed * deltaTime;
	bool moved = CanMoveTo(nextPosition);
	if (moved) robot.position = nextPosition;
	else SetRobotDirection(-robot.direction);
	UpdateRobotWalking(deltaTime, moved);
}

void ProcessCameraInput(float deltaTime)
{
	bool shift = glfwGetKey(mainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
		|| glfwGetKey(mainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	float sign = shift ? -1.0f : 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_Z) == GLFW_PRESS) camera.position.z -= sign * 4.0f * deltaTime;
	if (glfwGetKey(mainWindow, GLFW_KEY_X) == GLFW_PRESS) camera.position.x -= sign * 4.0f * deltaTime;
	// 카메라가 시선 중심과 겹치는 경우를 방지한다.
	if (glm::length(camera.position - camera.target) < 1.0f)
		camera.position = camera.target + glm::vec3(0,1,1);
}

void UpdateCamera(float deltaTime)
{
	ProcessCameraInput(deltaTime);
	glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(camera.orbitVelocityY * deltaTime), glm::vec3(0,1,0));
	camera.position = camera.target + glm::vec3(rotation * glm::vec4(camera.position - camera.target, 0));
}

void Update(float deltaTime)
{
	deltaTime = std::clamp(deltaTime, 0.0f, .05f);
	UpdateStage(deltaTime);
	UpdateCamera(deltaTime);
	// 작은 시간 간격으로 나눠 장애물 통과와 착지 누락을 방지한다.
	int steps = std::max(1, static_cast<int>(std::ceil(deltaTime * 240.0f)));
	for (int i = 0; i < steps; ++i)
	{
		UpdateRobotJump(deltaTime / steps);
		UpdateRobotMovement(deltaTime / steps);
	}
}

// 그리기: 현재 상태로 행렬을 만들고 같은 육면체 버퍼를 재사용한다.
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
	DrawMesh(cubeMesh);
}

void DrawPartScene(const glm::mat4& parent, const glm::vec3& center,
	const glm::vec3& size, const glm::vec3& color)
{
	DrawCubeScene(glm::scale(glm::translate(parent, center), size), color);
}

void DrawStageScene()
{
	const glm::vec3 colors[6] = {{.25f,.6f,.35f},{.25f,.35f,.65f},{.5f,.65f,.45f},
		{.65f,.6f,.4f},{.7f,.25f,.25f},{.3f,.65f,.7f}};
	for (int face = 0; face < 6; ++face)
	{
		glm::mat4 transform(1);
		if (face == Front)
		{
			if (frontOpenAmount >= .999f) continue;
			// 윗부분을 고정하고 높이를 줄이면 아래쪽이 위로 올라간다.
			glm::vec3 pivot(0, stageHeight, stageHalfDepth);
			transform = glm::translate(transform, pivot);
			transform = glm::scale(transform, glm::vec3(1, 1.0f - frontOpenAmount, 1));
			transform = glm::translate(transform, -pivot);
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
		glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(colors[face]));
		DrawMesh(stageMeshes[face]);
	}
}

void DrawObstacleScene()
{
	for (const Obstacle& obstacle : obstacles)
		DrawPartScene(glm::mat4(1), obstacle.position, obstacle.size, obstacle.color);
}

void DrawRobotLimb(const glm::mat4& robotTransform, const glm::vec3& joint,
	float angle, float length, float width, const glm::vec3& color)
{
	// 관절로 이동 → 관절에서 회전 → 관절 아래에 팔 또는 다리를 배치한다.
	glm::mat4 limbTransform = glm::translate(robotTransform, joint);
	limbTransform = glm::rotate(limbTransform, glm::radians(angle), glm::vec3(1,0,0));
	DrawPartScene(limbTransform, {0,-length * .5f,0}, {width,length,width}, color);
}

void DrawRobotScene()
{
	glm::mat4 robotTransform = glm::translate(glm::mat4(1), robot.position);
	robotTransform = glm::rotate(robotTransform, glm::radians(robot.rotateAngleY), glm::vec3(0,1,0));
	DrawPartScene(robotTransform, {0,1.15f,0}, {.65f,.8f,.38f}, {.2f,.6f,.95f});
	DrawPartScene(robotTransform, {0,1.82f,0}, {.5f,.5f,.5f}, {1,.8f,.5f});
	DrawPartScene(robotTransform, {0,1.8f,-.34f}, {.16f,.15f,.22f}, {1,.35f,.15f});
	float angle = robot.limbSwingAngle;
	DrawRobotLimb(robotTransform, {-.46f,1.5f,0}, -angle, .75f, .2f, {1,.2f,.2f});
	DrawRobotLimb(robotTransform, {.46f,1.5f,0}, angle, .75f, .2f, {.2f,.9f,.3f});
	DrawRobotLimb(robotTransform, {-.2f,.75f,0}, angle, .75f, .24f, {.8f,.2f,.9f});
	DrawRobotLimb(robotTransform, {.2f,.75f,0}, -angle, .75f, .24f, {1,.8f,.15f});
}

void DrawScene()
{
	glClearColor(.08f,.08f,.11f,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);
	glm::mat4 view = glm::lookAt(camera.position, camera.target, glm::vec3(0,1,0));
	float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
	glm::mat4 projection = glm::perspective(glm::radians(50.0f), aspect, .1f, 100.0f);
	SetTransformUniforms(shaderProgram, glm::mat4(1), view, projection);
	DrawStageScene();
	DrawObstacleScene();
	DrawRobotScene();
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
	// 속도 조절만 길게 눌렀을 때 반복 입력을 받는다.
	if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
		robot.walkSpeed = std::min(3.5f, robot.walkSpeed + .25f);
	if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
		robot.walkSpeed = std::max(.5f, robot.walkSpeed - .25f);
	if (action != GLFW_PRESS) return;
	if (key == GLFW_KEY_W) SetRobotDirection({0,0,-1});
	if (key == GLFW_KEY_S) SetRobotDirection({0,0,1});
	if (key == GLFW_KEY_A) SetRobotDirection({-1,0,0});
	if (key == GLFW_KEY_D) SetRobotDirection({1,0,0});
	if (key == GLFW_KEY_J) StartJump();
	if (key == GLFW_KEY_O) isFrontOpen = !isFrontOpen;
	if (key == GLFW_KEY_Y)
	{
		float velocity = (mods & GLFW_MOD_SHIFT) ? -30.0f : 30.0f;
		camera.orbitVelocityY = camera.orbitVelocityY == velocity ? 0.0f : velocity;
	}
	if (key == GLFW_KEY_I) ResetScene();
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
	DeleteMesh(cubeMesh);
	for (Mesh& mesh : stageMeshes) DeleteMesh(mesh);
	if (shaderProgram) glDeleteProgram(shaderProgram);
}
