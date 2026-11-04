#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <ctime>

// 로봇의 위치는 두 발 아래의 바닥 중심이다.
// 로봇의 위치와 진행 방향 및 걷기와 점프의 현재 상태를 저장한다.
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

// 장애물의 중심 위치와 실제 크기 및 표시 색상을 저장한다.
struct Obstacle
{
	glm::vec3 position;
	glm::vec3 size;
	glm::vec3 color;
};

// 카메라 위치와 바라보는 위치 및 공전 속도를 저장한다.
struct Camera
{
	glm::vec3 position{0.0f, 4.8f, 13.5f};
	glm::vec3 target{0.0f, 2.3f, 0.0f};
	float orbitVelocityY = 0.0f;
};

enum StageFace { Left, Right, Bottom, Top, Back, Front };
GLFWwindow* mainWindow = nullptr;
// 셰이더 프로그램과 재사용할 GPU 메시 및 장면 상태를 관리한다.
GLuint shaderProgram = 0;
Mesh cubeMesh;
std::array<Mesh, 6> stageMeshes;
Robot robot;
Camera camera;
std::array<Obstacle, 3> obstacles;
int framebufferWidth = 1000;
int framebufferHeight = 800;

// 방 크기와 로봇 충돌 범위 및 점프 속도와 중력을 정한다.
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

// 윈도우와 OpenGL을 초기화하고 입력 처리와 화면 갱신을 반복한 뒤 자원을 정리한다.
int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	// GLFW를 초기화하고 실패하면 프로그램 시작을 중단한다.
	if (!glfwInit()) return -1;
	// OpenGL 버전과 Core 프로파일을 지정한다.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// 지정한 해상도의 윈도우와 OpenGL 컨텍스트를 생성한다.
	mainWindow = glfwCreateWindow(1000, 800, "Practice23", nullptr, nullptr);
	if (!mainWindow) { glfwTerminate(); return -1; }
	// 현재 스레드에서 사용할 OpenGL 컨텍스트를 선택한다.
	glfwMakeContextCurrent(mainWindow);
	// 컨텍스트 생성 뒤 GLEW를 초기화하여 OpenGL 함수 주소를 불러온다.
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { glfwDestroyWindow(mainWindow); glfwTerminate(); return -1; }
	glfwSwapInterval(1);
	// 키보드 입력이 발생할 때 호출할 함수를 등록한다.
	glfwSetKeyCallback(mainWindow, keyCallBack);
	glfwSetFramebufferSizeCallback(mainWindow, framebufferCallBack);
	glfwGetFramebufferSize(mainWindow, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);
	// 깊이 버퍼를 사용하여 가까운 표면이 먼 표면을 가리도록 한다.
	glEnable(GL_DEPTH_TEST);
	// 정점 순서에 따른 뒷면 컬링을 사용한다.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	// GLSL 파일을 읽어 컴파일하고 실행할 셰이더 프로그램으로 연결한다.
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (!shaderProgram) { Cleanup(); glfwDestroyWindow(mainWindow); glfwTerminate(); return -1; }
	InitBuffer();
	ResetScene();

	double lastTime = glfwGetTime();
	// 종료 요청 전까지 상태 갱신과 렌더링 및 이벤트 처리를 반복한다.
	while (!glfwWindowShouldClose(mainWindow))
	{
		double currentTime = glfwGetTime();
		Update(static_cast<float>(currentTime - lastTime));
		lastTime = currentTime;
		DrawScene();
		// 완성된 뒤쪽 버퍼를 화면에 표시한다.
		glfwSwapBuffers(mainWindow);
		// 대기 중인 키보드와 마우스 및 창 이벤트를 처리한다.
		glfwPollEvents();
	}
	Cleanup();
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
	return 0;
}

// 초기화: 정점은 한 번만 버퍼에 저장한다.
// 실습에서 사용할 정점과 GPU 버퍼를 처음 한 번 준비한다.
void InitBuffer()
{
	InitCubeBuffer();
	InitStageBuffer();
}

// 여러 부품이 재사용할 정육면체 정점과 색상을 VAO와 VBO에 저장한다.
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

// 여섯 면의 안쪽이 정면이 되도록 방을 구성하여 카메라에 따라 가까운 벽이 컬링되게 한다.
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

// 로봇 시작 위치와 다른 장애물을 피해서 크기가 다른 장애물 세 개를 무작위로 배치한다.
void InitObstacles()
{
	// 요청한 최소와 최대 범위 사이의 난수를 만드는 함수를 준비한다.
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
			// 로봇 시작 위치와 겹치거나 다른 장애물과 너무 가까운 배치는 다시 뽑는다.
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

// 로봇과 카메라 및 문을 초기화하고 장애물 위치를 다시 생성한다.
void ResetScene()
{
	robot = Robot{};
	camera = Camera{};
	isFrontOpen = false;
	frontOpenAmount = 0.0f;
	InitObstacles();
}

// 충돌: 로봇의 바닥 면적과 장애물의 바닥 면적을 비교한다.
// 로봇과 장애물의 xz 평면 범위가 겹치는지 검사한다.
bool OverlapsObstacle(const glm::vec3& position, const Obstacle& obstacle)
{
	glm::vec3 half = obstacle.size * .5f;
	return std::abs(position.x - obstacle.position.x) < half.x + robotHalfWidth
		&& std::abs(position.z - obstacle.position.z) < half.z + robotHalfDepth;
}

// 현재 발 아래에 있는 바닥 또는 올라선 장애물의 지지 높이를 구한다.
float GetSupportHeight(const glm::vec3& position)
{
	// 기본 바닥 높이에서 시작해 발 아래의 유효한 장애물 윗면 높이를 찾는다.
	float height = 0.0f;
	for (const Obstacle& obstacle : obstacles)
	{
		float top = obstacle.position.y + obstacle.size.y * .5f;
		if (OverlapsObstacle(position, obstacle) && position.y >= top - .001f)
			height = std::max(height, top);
	}
	return height;
}

// 다음 위치가 방 안에 있고 장애물의 옆면과 충돌하지 않는지 검사한다.
bool CanMoveTo(const glm::vec3& position)
{
	// 로봇의 여유 폭까지 고려하여 방 경계 밖으로 이동하는 것을 막는다.
	if (std::abs(position.x) > stageHalfWidth - robotHalfWidth
		|| std::abs(position.z) > stageHalfDepth - robotHalfDepth) return false;
	for (const Obstacle& obstacle : obstacles)
	{
		float top = obstacle.position.y + obstacle.size.y * .5f;
		float bottom = obstacle.position.y - obstacle.size.y * .5f;
		// 발이 장애물 위보다 낮으면 옆면 충돌로 처리하고 충분히 높으면 위로 지나가게 한다.
		if (OverlapsObstacle(position, obstacle) && position.y < top - .001f
			&& position.y + robotHeight > bottom) return false;
	}
	return true;
}

// 이동 방향을 저장하고 코가 진행 방향을 향하도록 로봇 전체 회전 각도를 구한다.
void SetRobotDirection(const glm::vec3& direction)
{
	robot.direction = direction;
	// 기본 앞쪽인 음의 z축을 현재 이동 방향으로 돌릴 y축 각도를 계산한다.
	robot.rotateAngleY = glm::degrees(std::atan2(-direction.x, -direction.z));
}

// 땅이나 장애물 위에 서 있을 때만 위쪽 초기 속도를 주어 점프를 시작한다.
void StartJump()
{
	// 이미 공중에 있으면 추가 점프 입력을 무시한다.
	if (!robot.isGrounded) return;
	robot.isGrounded = false;
	robot.jumpVelocity = jumpSpeed;
}

// 상태 갱신: 문, 점프, 이동, 팔다리, 카메라를 각각 처리한다.
// 앞면이 윗부분에 모이도록 열림 비율을 목표 값까지 증가시키거나 감소시킨다.
void UpdateStage(float deltaTime)
{
	// 열림 토글에 따라 앞면의 스케일 진행 비율 목표를 선택한다.
	// 문 열기와 닫기에 따라 현재 비율을 목표까지 이동시킨다.
	float target = isFrontOpen ? 1.0f : 0.0f;
	if (frontOpenAmount < target) frontOpenAmount = std::min(target, frontOpenAmount + deltaTime);
	if (frontOpenAmount > target) frontOpenAmount = std::max(target, frontOpenAmount - deltaTime);
}

// 중력으로 점프 높이를 갱신하고 내려올 때 바닥이나 장애물 윗면에 착지시킨다.
void UpdateRobotJump(float deltaTime)
{
	// 장애물에서 걸어나왔는지 검사하고 발 아래 지지가 없으면 하강을 시작한다.
	float support = GetSupportHeight(robot.position);
	if (robot.isGrounded && robot.position.y > support + .001f) robot.isGrounded = false;
	if (robot.isGrounded) return;
	float previousY = robot.position.y;
	// 중력으로 위쪽 속도를 줄이고 이후 음의 속도로 하강시킨다.
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
			// 착지한 표면 높이에 발을 맞추고 점프 속도를 초기화한다.
			robot.position.y = landingHeight;
			robot.jumpVelocity = 0.0f;
			robot.isGrounded = true;
		}
	}
}

// 이동 속도에 따라 걷는 주기와 최대 흔들림 각도를 바꾸어 팔다리 각도를 계산한다.
void UpdateRobotWalking(float deltaTime, bool moved)
{
	if (!moved) return;
	robot.walkPhase = std::fmod(robot.walkPhase + robot.walkSpeed * 220.0f * deltaTime, 360.0f);
	// 빠르게 걸을수록 팔다리의 최대 회전 각도도 크게 만든다.
	float amplitude = 12.0f + robot.walkSpeed * 9.0f;
	robot.limbSwingAngle = amplitude * std::sin(glm::radians(robot.walkPhase));
}

// 진행 방향으로 한 걸음 이동하고 막히면 그 자리에서 반대 방향으로 돌아선다.
void UpdateRobotMovement(float deltaTime)
{
	// 진행 방향과 속도 및 경과 시간으로 다음 후보 위치를 만든다.
	glm::vec3 nextPosition = robot.position + robot.direction * robot.walkSpeed * deltaTime;
	bool moved = CanMoveTo(nextPosition);
	if (moved) robot.position = nextPosition;
	// 이동할 수 없으면 위치를 유지하고 반대 방향으로 돌아선다.
	else SetRobotDirection(-robot.direction);
	UpdateRobotWalking(deltaTime, moved);
}

// x와 z 키를 누르는 동안 카메라 위치를 이동하고 Shift로 이동 방향을 반대로 바꾼다.
void ProcessCameraInput(float deltaTime)
{
	// 카메라 이동 키에서 Shift 여부로 앞뒤와 좌우 이동 방향을 바꾼다.
	bool shift = glfwGetKey(mainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
		|| glfwGetKey(mainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	float sign = shift ? -1.0f : 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_Z) == GLFW_PRESS) camera.position.z -= sign * 4.0f * deltaTime;
	if (glfwGetKey(mainWindow, GLFW_KEY_X) == GLFW_PRESS) camera.position.x -= sign * 4.0f * deltaTime;
	// 카메라가 시선 중심과 겹치는 경우를 방지한다.
	if (glm::length(camera.position - camera.target) < 1.0f)
		camera.position = camera.target + glm::vec3(0,1,1);
}

// 이동 입력을 처리하고 카메라를 장면 중심의 y축 주위로 공전시킨다.
void UpdateCamera(float deltaTime)
{
	ProcessCameraInput(deltaTime);
	// 카메라와 목표 사이 벡터를 회전하여 목표를 중심으로 공전시킨다.
	glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(camera.orbitVelocityY * deltaTime), glm::vec3(0,1,0));
	camera.position = camera.target + glm::vec3(rotation * glm::vec4(camera.position - camera.target, 0));
}

// 문과 카메라를 갱신하고 짧은 시간 간격으로 점프와 이동을 함께 처리한다.
void Update(float deltaTime)
{
	// 긴 프레임에서 이동이 과도하게 건너뛰지 않도록 경과 시간을 제한한다.
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
// 한 부품의 모델 행렬과 색상만 바꾸어 동일한 육면체 버퍼를 재사용한다.
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
	DrawMesh(cubeMesh);
}

// 부모 변환에 부품 중심으로의 이동과 크기 조절을 결합하여 그린다.
void DrawPartScene(const glm::mat4& parent, const glm::vec3& center,
	const glm::vec3& size, const glm::vec3& color)
{
	DrawCubeScene(glm::scale(glm::translate(parent, center), size), color);
}

// 면마다 다른 색으로 방을 그리고 앞면에만 윗모서리 기준 스케일을 적용한다.
void DrawStageScene()
{
	const glm::vec3 colors[6] = {{.25f,.6f,.35f},{.25f,.35f,.65f},{.5f,.65f,.45f},
		{.65f,.6f,.4f},{.7f,.25f,.25f},{.3f,.65f,.7f}};
	for (int face = 0; face < 6; ++face)
	{
		glm::mat4 transform(1);
		if (face == Front)
		{
			// 완전히 열린 앞면은 높이가 거의 없는 면을 그리는 대신 건너뛴다.
			if (frontOpenAmount >= .999f) continue;
			// 윗부분을 고정하고 높이를 줄이면 아래쪽이 위로 올라간다.
			glm::vec3 pivot(0, stageHeight, stageHalfDepth);
			transform = glm::translate(transform, pivot);
			// 윗부분을 고정한 상태에서 높이를 줄여 앞면의 아래쪽이 위로 올라오게 한다.
			transform = glm::scale(transform, glm::vec3(1, 1.0f - frontOpenAmount, 1));
			transform = glm::translate(transform, -pivot);
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
		glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(colors[face]));
		DrawMesh(stageMeshes[face]);
	}
}

// 저장된 위치와 크기 및 색상으로 장애물 세 개를 그린다.
void DrawObstacleScene()
{
	for (const Obstacle& obstacle : obstacles)
		DrawPartScene(glm::mat4(1), obstacle.position, obstacle.size, obstacle.color);
}

// 어깨나 골반 관절을 기준으로 x축 회전한 뒤 그 아래에 팔 또는 다리를 배치한다.
void DrawRobotLimb(const glm::mat4& robotTransform, const glm::vec3& joint,
	float angle, float length, float width, const glm::vec3& color)
{
	// 관절로 이동 → 관절에서 회전 → 관절 아래에 팔 또는 다리를 배치한다.
	glm::mat4 limbTransform = glm::translate(robotTransform, joint);
	limbTransform = glm::rotate(limbTransform, glm::radians(angle), glm::vec3(1,0,0));
	DrawPartScene(limbTransform, {0,-length * .5f,0}, {width,length,width}, color);
}

// 로봇 전체의 위치와 진행 방향을 적용한 뒤 몸통과 머리 및 팔다리를 조립한다.
void DrawRobotScene()
{
	glm::mat4 robotTransform = glm::translate(glm::mat4(1), robot.position);
	robotTransform = glm::rotate(robotTransform, glm::radians(robot.rotateAngleY), glm::vec3(0,1,0));
	// 몸통과 머리 및 코를 로봇 전체 변환에 붙여 배치한다.
	DrawPartScene(robotTransform, {0,1.15f,0}, {.65f,.8f,.38f}, {.2f,.6f,.95f});
	DrawPartScene(robotTransform, {0,1.82f,0}, {.5f,.5f,.5f}, {1,.8f,.5f});
	DrawPartScene(robotTransform, {0,1.8f,-.34f}, {.16f,.15f,.22f}, {1,.35f,.15f});
	// 같은 쪽 팔과 다리 및 양쪽 팔다리가 서로 반대로 흔들리도록 부호를 나눈다.
	float angle = robot.limbSwingAngle;
	DrawRobotLimb(robotTransform, {-.46f,1.5f,0}, -angle, .75f, .2f, {1,.2f,.2f});
	DrawRobotLimb(robotTransform, {.46f,1.5f,0}, angle, .75f, .2f, {.2f,.9f,.3f});
	DrawRobotLimb(robotTransform, {-.2f,.75f,0}, angle, .75f, .24f, {.8f,.2f,.9f});
	DrawRobotLimb(robotTransform, {.2f,.75f,0}, -angle, .75f, .24f, {1,.8f,.15f});
}

// 원근 투영과 카메라를 설정한 뒤 방과 장애물 및 로봇을 같은 장면에 그린다.
void DrawScene()
{
	glClearColor(.08f,.08f,.11f,1);
	// 색상과 깊이 버퍼를 지워 이전 프레임의 결과를 제거한다.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 이후 그리기에 사용할 셰이더 프로그램을 활성화한다.
	glUseProgram(shaderProgram);
	glm::mat4 view = glm::lookAt(camera.position, camera.target, glm::vec3(0,1,0));
	float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
	glm::mat4 projection = glm::perspective(glm::radians(50.0f), aspect, .1f, 100.0f);
	SetTransformUniforms(shaderProgram, glm::mat4(1), view, projection);
	DrawStageScene();
	DrawObstacleScene();
	DrawRobotScene();
}

// 이동 방향과 속도 및 점프와 문 및 카메라 공전 입력을 처리한다.
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
	// 속도 조절만 길게 눌렀을 때 반복 입력을 받는다.
	// 더하기 키를 누르면 걷기 속도를 최대 제한 안에서 증가시킨다.
	if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
		robot.walkSpeed = std::min(3.5f, robot.walkSpeed + .25f);
	// 빼기 키를 누르면 걷기 속도를 최소 제한 안에서 감소시킨다.
	if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
		robot.walkSpeed = std::max(.5f, robot.walkSpeed - .25f);
	// 반복 입력은 속도 조절까지만 허용하고 토글과 점프는 처음 누를 때만 처리한다.
	if (action != GLFW_PRESS) return;
	// 앞뒤와 좌우 키로 로봇이 계속 걸어갈 방향을 바꾼다.
	if (key == GLFW_KEY_W) SetRobotDirection({0,0,-1});
	if (key == GLFW_KEY_S) SetRobotDirection({0,0,1});
	if (key == GLFW_KEY_A) SetRobotDirection({-1,0,0});
	if (key == GLFW_KEY_D) SetRobotDirection({1,0,0});
	// 바닥이나 장애물 위에 있을 때 점프를 시작한다.
	if (key == GLFW_KEY_J) StartJump();
	// 무대 앞면의 열기와 닫기를 전환한다.
	if (key == GLFW_KEY_O) isFrontOpen = !isFrontOpen;
	// 카메라 공전의 양방향 회전과 정지를 선택한다.
	if (key == GLFW_KEY_Y)
	{
		float velocity = (mods & GLFW_MOD_SHIFT) ? -30.0f : 30.0f;
		camera.orbitVelocityY = camera.orbitVelocityY == velocity ? 0.0f : velocity;
	}
	// 로봇과 카메라 및 장애물과 문을 다시 초기화한다.
	if (key == GLFW_KEY_I) ResetScene();
	// 프로그램 종료를 요청한다.
	if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// 실제 화면 크기를 저장하고 뷰포트를 갱신해 창 크기 변경에 대응한다.
void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	framebufferWidth = std::max(width, 1);
	framebufferHeight = std::max(height, 1);
	glViewport(0, 0, width, height);
}

// OpenGL 컨텍스트가 살아 있는 동안 생성한 버퍼와 셰이더 프로그램을 해제한다.
void Cleanup()
{
	DeleteMesh(cubeMesh);
	for (Mesh& mesh : stageMeshes) DeleteMesh(mesh);
	if (shaderProgram) glDeleteProgram(shaderProgram);
}
