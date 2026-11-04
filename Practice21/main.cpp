#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

// 공과 육면체의 이동 및 방의 개폐 상태를 구분해 관리한다.
// 공의 위치와 일정한 이동 속도 및 반지름과 탈출 상태를 관리한다.
struct Ball
{
	glm::vec3 position;
	glm::vec3 velocity;
	float radius;
	glm::vec3 color;
	bool escaped = false;
};

// 육면체의 위치와 중력으로 변하는 속도 및 크기와 활성 상태를 저장한다.
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
// 셰이더 프로그램과 재사용할 GPU 메시 및 장면 상태를 관리한다.
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
// 육면체에 적용할 중력과 공에 적용할 일정한 속력을 따로 설정한다.
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
	mainWindow = glfwCreateWindow(900, 800, "Practice21", nullptr, nullptr);
	if (!mainWindow)
	{
		glfwTerminate();
		return -1;
	}
	// 현재 스레드에서 사용할 OpenGL 컨텍스트를 선택한다.
	glfwMakeContextCurrent(mainWindow);
	glfwSwapInterval(1);
	// 키보드 입력이 발생할 때 호출할 함수를 등록한다.
	glfwSetKeyCallback(mainWindow, keyCallBack);
	glfwSetMouseButtonCallback(mainWindow, mouseCallBack);
	glfwSetCursorPosCallback(mainWindow, cursorCallBack);
	glfwSetFramebufferSizeCallback(mainWindow, framebufferCallBack);
	// 컨텍스트 생성 뒤 GLEW를 초기화하여 OpenGL 함수 주소를 불러온다.
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return -1;
	}
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
	// 종료 요청 전까지 상태 갱신과 렌더링 및 이벤트 처리를 반복한다.
	while (!glfwWindowShouldClose(mainWindow))
	{
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - lastFrameTime);
		lastFrameTime = currentTime;
		Update(deltaTime);
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

// 안쪽이 정면인 다섯 벽의 정점을 만들고 면마다 별도 버퍼로 저장한다.
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

// 위도와 경도를 나누어 구 표면을 삼각형으로 만들고 공이 재사용할 버퍼를 저장한다.
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

// 여러 부품이 재사용할 정육면체 정점과 색상을 VAO와 VBO에 저장한다.
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

// 실습에서 사용할 정점과 GPU 버퍼를 처음 한 번 준비한다.
void InitBuffer()
{
	InitRoomBuffer();
	InitSphereBuffer();
	InitCubeBuffer();
}

// 최대 다섯 개까지 공을 추가하고 무작위 이동 방향을 일정한 속력으로 정규화한다.
void AddBall()
{
	if (balls.size() >= 5) return;
	int index = static_cast<int>(balls.size());
	const glm::vec3 colors[5] = {{0.2f,0.3f,1.0f}, {0.2f,0.8f,1.0f}, {0.6f,0.3f,1.0f}, {0.1f,0.9f,0.6f}, {1.0f,0.65f,0.15f}};
	auto random = []() { return static_cast<float>(std::rand()) / RAND_MAX; };
	glm::vec3 position(-1.3f + 2.6f * random(), 0.3f + 1.0f * random(), -1.8f + 3.6f * random());
	// 공마다 무작위 방향을 고르되 정규화하여 같은 속력을 갖게 한다.
	glm::vec3 velocity((random() < 0.5f ? -1 : 1) * (1.3f + random()), 1.2f + random(), -1.5f + 3.0f * random());
	velocity = glm::normalize(velocity) * ballSpeed;
	balls.push_back({position, velocity, 0.16f + 0.015f * index, colors[index]});
}

// 방과 열리는 면을 초기화하고 공 한 개 및 바닥의 육면체 세 개를 다시 배치한다.
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

// 원래 아래쪽 면이 현재 충돌 가능한 상태인지 공통 면 판정으로 확인한다.
bool IsFloorSolid()
{
	return IsFaceSolid(1, -1);
}

// 열리는 각도와 선택된 면의 축 및 부호를 비교해 해당 경계의 충돌 여부를 결정한다.
bool IsFaceSolid(int axis, int sign)
{
	// 각 벽이 어느 축의 양쪽 경계에 해당하는지 면 번호와 연결한다.
	const int axes[5] = {0, 0, 1, 1, 2};
	const int signs[5] = {-1, 1, -1, 1, -1};
	return floorOpenAngle < 3.0f || axes[openFace] != axis || signs[openFace] != sign;
}

// 객체 전체가 충돌 해제된 면의 바깥으로 빠져나갔는지 검사한다.
bool HasEscaped(const glm::vec3& position, const glm::vec3& halfSize)
{
	for (int axis = 0; axis < 3; ++axis)
	{
		if (!IsFaceSolid(axis, -1) && position[axis] + halfSize[axis] < -roomHalfSize[axis]) return true;
		if (!IsFaceSolid(axis, 1) && position[axis] - halfSize[axis] > roomHalfSize[axis]) return true;
	}
	return false;
}

// 방의 y축 회전과 z축 기울기를 결합한 회전 행렬을 만든다.
glm::mat4 GetRoomRotation()
{
	glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(roomRotateAngleY), glm::vec3(0, 1, 0));
	return glm::rotate(rotation, glm::radians(roomRotateAngleZ), glm::vec3(0, 0, 1));
}

// z 키와 Shift 조합으로 방 전체를 카메라 앞뒤 방향으로 이동시킨다.
void ProcessInput(float deltaTime)
{
	bool shift = glfwGetKey(mainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	if (glfwGetKey(mainWindow, GLFW_KEY_Z) == GLFW_PRESS) roomMoveZ += (shift ? -1.0f : 1.0f) * 2.0f * deltaTime;
	roomMoveZ = std::clamp(roomMoveZ, -6.0f, 3.0f);
}

// 구와 육면체의 겹침을 해소하고 공의 속력을 유지하며 충돌면에서 이동 방향을 반사한다.
void ResolveBallBox(Ball& ball, const Box& box)
{
	if (!box.active || box.escaped || ball.escaped) return;
	glm::vec3 minimum = box.position - box.size * 0.5f;
	glm::vec3 maximum = box.position + box.size * 0.5f;
	// 구 중심에서 육면체 표면의 가장 가까운 점을 구한다.
	glm::vec3 nearest = glm::clamp(ball.position, minimum, maximum);
	glm::vec3 difference = ball.position - nearest;
	float distance = glm::length(difference);
	if (distance >= ball.radius) return;
	glm::vec3 normal;
	float penetration;
	// 구 중심이 육면체 밖에 있으면 가까운 점과의 차이로 충돌 법선을 구한다.
	if (distance > 0.00001f)
	{
		normal = difference / distance;
		penetration = ball.radius - distance;
	}
	else
	{
		// 구 중심이 육면체 내부에 들어간 경우 가장 가까운 면으로 밀어낸다.
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
	// 겹친 깊이만큼 공을 표면 밖으로 옮겨 연속적인 끼임을 방지한다.
	ball.position += normal * penetration;
	// 표면을 향해 들어갈 때만 법선 기준으로 방향을 반사해 속력을 유지한다.
	if (glm::dot(ball.velocity, normal) < 0.0f)
		ball.velocity = glm::reflect(ball.velocity, normal);
}

// 육면체에는 중력과 마찰을 적용하고 공은 등속 이동시키며 벽과 장애물 충돌을 처리한다.
void UpdatePhysics(float deltaTime, const glm::vec3& gravity)
{
	// 육면체는 중력과 바닥 마찰을 받으며 열려 있는 면으로 빠져나갈 수 있다.
	for (Box& box : boxes)
	{
		if (!box.active) continue;
		box.velocity += gravity * deltaTime;
		box.position += box.velocity * deltaTime;
		glm::vec3 half = box.size * 0.5f;
		// 완전히 방 밖으로 나간 육면체는 다시 벽에 붙잡히지 않도록 탈출 상태를 저장한다.
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
			// 바닥에 닿아 있는 육면체의 가로 속도를 감쇠하여 마찰을 표현한다.
			if (IsFloorSolid() && box.position.y - half.y <= -roomHalfSize.y + 0.001f)
			{
				box.velocity.x *= std::exp(-0.5f * deltaTime);
				box.velocity.z *= std::exp(-0.5f * deltaTime);
			}
		}
		// 너무 멀어진 육면체는 비활성화하여 더 이상 갱신하거나 그리지 않는다.
		if (glm::length(box.position) > 20.0f) box.active = false;
	}
	// 공은 중력이나 마찰로 속력이 변하지 않고 현재 속도로만 이동한다.
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
				// 공의 중심이 아닌 반지름까지 고려한 벽 접촉 범위를 구한다.
				float minimum = -roomHalfSize[axis] + ball.radius;
				float maximum = roomHalfSize[axis] - ball.radius;
				// 닫힌 음의 축 경계에서는 위치를 보정하고 해당 속도 성분을 양수로 반전한다.
				if (ball.position[axis] < minimum && IsFaceSolid(axis, -1))
				{
					ball.position[axis] = minimum;
					ball.velocity[axis] = std::abs(ball.velocity[axis]);
				}
				// 닫힌 양의 축 경계에서는 위치를 보정하고 해당 속도 성분을 음수로 반전한다.
				if (ball.position[axis] > maximum && IsFaceSolid(axis, 1))
				{
					ball.position[axis] = maximum;
					ball.velocity[axis] = -std::abs(ball.velocity[axis]);
				}
			}
			for (const Box& box : boxes) ResolveBallBox(ball, box);
		}
	}
	// 너무 멀리 나간 공을 목록에서 지워 추가 가능한 공 개수를 확보한다.
	balls.erase(std::remove_if(balls.begin(), balls.end(), [](const Ball& ball) { return glm::length(ball.position) > 20.0f; }), balls.end());
}

// 방 회전과 면 열기 각도를 갱신하고 짧은 시간 간격으로 객체 이동과 충돌을 계산한다.
void Update(float deltaTime)
{
	// 긴 프레임에서 이동이 과도하게 건너뛰지 않도록 경과 시간을 제한한다.
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	ProcessInput(deltaTime);
	roomRotateAngleY = std::fmod(roomRotateAngleY + roomRotateVelocityY * deltaTime, 360.0f);
	// 열기와 닫기의 목표 각도를 선택하고 현재 각도를 서서히 접근시킨다.
	float target = floorOpen ? 90.0f : 0.0f;
	if (floorOpenAngle < target) floorOpenAngle = std::min(floorOpenAngle + 75.0f * deltaTime, target);
	if (floorOpenAngle > target) floorOpenAngle = std::max(floorOpenAngle - 75.0f * deltaTime, target);
	// 방이 기울어져도 중력은 화면의 아래쪽을 향한다.
	glm::vec3 gravity = glm::transpose(glm::mat3(GetRoomRotation())) * glm::vec3(0, -gravityStrength, 0);
	// 한 프레임을 작은 시간 간격으로 나누어 벽을 뚫고 지나가는 현상을 줄인다.
	int steps = std::max(1, static_cast<int>(std::ceil(deltaTime * 240.0f)));
	for (int i = 0; i < steps; ++i) UpdatePhysics(deltaTime / steps, gravity);
}

// 현재 객체의 모델 행렬과 색상을 셰이더에 전달한다.
void SetObjectTransform(const glm::mat4& transform, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
}

// 방 전체를 회전시키고 선택된 한 면만 모서리 기준으로 열어 그린다.
void DrawRoomScene(const glm::mat4& roomTransform)
{
	const glm::vec3 colors[5] = {{0.35f,0.42f,0.55f}, {0.55f,0.38f,0.42f},
		{0.4f,0.5f,0.35f}, {0.6f,0.55f,0.4f}, {0.5f,0.52f,0.56f}};
	for (int face = 0; face < 5; ++face)
	{
		glm::mat4 transform = roomTransform;
		// 선택된 면에만 모서리 기준 회전을 적용하고 다른 벽은 방 변환만 사용한다.
		if (face == openFace)
		{
			// 각 면이 밖으로 열릴 때 사용할 회전 중심 모서리를 지정한다.
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

// 방의 부모 변환에 공의 위치와 반지름을 결합하여 그린다.
void DrawSphereScene(const Ball& ball, const glm::mat4& roomTransform)
{
	glm::mat4 transform = glm::translate(roomTransform, ball.position);
	SetObjectTransform(glm::scale(transform, glm::vec3(ball.radius)), ball.color);
	DrawMesh(sphereMesh);
}

// 활성 상태인 육면체에 방의 변환과 위치 및 크기를 적용하여 그린다.
void DrawCubeScene(const Box& box, const glm::mat4& roomTransform)
{
	if (!box.active) return;
	glm::mat4 transform = glm::translate(roomTransform, box.position);
	SetObjectTransform(glm::scale(transform, box.size), box.color);
	DrawMesh(cubeMesh);
}

// 원근 투영과 고정 카메라를 설정하고 같은 방 변환으로 벽과 모든 객체를 그린다.
void DrawScene()
{
	glClearColor(0.02f, 0.02f, 0.025f, 1.0f);
	// 색상과 깊이 버퍼를 지워 이전 프레임의 결과를 제거한다.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 이후 그리기에 사용할 셰이더 프로그램을 활성화한다.
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

// 공 추가와 방 회전 및 가장 아래를 향하는 면의 열기와 초기화를 처리한다.
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action != GLFW_PRESS) return;
	// 새 공을 추가하되 동시에 존재하는 최대 개수는 유지한다.
	if (key == GLFW_KEY_B) AddBall();
	// 방의 y축 회전을 시작하고 Shift로 방향을 바꾸며 같은 입력으로 정지한다.
	if (key == GLFW_KEY_Y)
	{
		float velocity = (mods & GLFW_MOD_SHIFT) ? 35.0f : -35.0f;
		roomRotateVelocityY = roomRotateVelocityY == velocity ? 0.0f : velocity;
	}
	// 닫힌 상태에서 현재 가장 아래를 향하는 면을 선택하고 열기와 닫기를 전환한다.
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
				// 회전된 바깥 법선의 y값이 가장 작은 면을 아래쪽 면으로 판단한다.
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
	// 공과 육면체 및 방의 위치와 열린 면을 초기 상태로 되돌린다.
	if (key == GLFW_KEY_R) ResetScene();
	// 윈도우 종료를 요청한다.
	if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// 왼쪽 버튼을 누른 동안만 드래그를 활성화하고 시작 마우스 위치를 저장한다.
void mouseCallBack(GLFWwindow* window, int button, int action, int mods)
{
	(void)mods;
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;
	mouseDragging = action == GLFW_PRESS;
	double y;
	glfwGetCursorPos(window, &lastMouseX, &y);
}

// 드래그의 좌우 이동량에 비례해 방을 z축 기준으로 기울인다.
void cursorCallBack(GLFWwindow* window, double x, double y)
{
	(void)window;
	(void)y;
	if (!mouseDragging) return;
	// 드래그한 가로 픽셀 차이를 z축 회전 각도로 환산한다.
	roomRotateAngleZ = std::fmod(roomRotateAngleZ - static_cast<float>(x - lastMouseX) * 0.35f, 360.0f);
	lastMouseX = x;
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
	for (Mesh& mesh : roomMeshes) DeleteMesh(mesh);
	DeleteMesh(sphereMesh);
	DeleteMesh(cubeMesh);
	glDeleteProgram(shaderProgram);
}
