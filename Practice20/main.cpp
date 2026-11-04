#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

// 카메라 공전 각도와 거리 및 투영 모드를 저장한다.
struct Camera
{
	glm::vec3 target{0.0f, 0.65f, 0.0f};
	float orbitAngleY = 35.0f;
	float elevation = 35.0f;
	float distance = 11.0f;
	float fovDegrees = 45.0f;
	bool perspective = true;
};

// 상자의 번호와 경로 시작 거리 및 크기와 색상을 저장한다.
struct Parcel
{
	int number;
	float pathOffset;
	glm::vec3 size;
	glm::vec3 color;
};

// 경로 위의 위치와 그 방향에 맞는 y축 회전 각도를 묶는다.
struct PathPose
{
	glm::vec3 position;
	float angleY;
};

// 고정된 난간과 지지대의 최종 행렬과 색상을 저장한다.
struct CubePart
{
	glm::mat4 transform;
	glm::vec3 color;
};

// 셰이더 프로그램과 재사용할 GPU 메시 및 장면 상태를 관리한다.
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
// 벨트 높이와 폭 및 직선과 코너의 경로 크기를 설정한다.
const float beltHeight = 1.1f;
const float beltHalfWidth = 0.55f;
const float startX = -2.4f;
const float startZ = 2.8f;
const float firstLength = 4.1f;
const float cornerRadius = 0.8f;
const float secondLength = 4.5f;
const float beltSpeed = 0.85f;
// 줄무늬와 상자가 함께 사용할 현재 이동 거리와 이동 토글을 저장한다.
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

// 윈도우와 OpenGL을 초기화하고 입력 처리와 화면 갱신을 반복한 뒤 자원을 정리한다.
int main()
{
	// GLFW를 초기화하고 실패하면 프로그램 시작을 중단한다.
	if (!glfwInit()) return -1;
	// OpenGL 버전과 Core 프로파일을 지정한다.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// 지정한 해상도의 윈도우와 OpenGL 컨텍스트를 생성한다.
	mainWindow = glfwCreateWindow(900, 800, "Practice20", nullptr, nullptr);
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
	std::cout << "b: belt start/stop | +/-: zoom | p/P: perspective/orthographic\n"
		<< "Left/Right: camera orbit | r: reset | q: quit" << std::endl;
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

// 첫 직선과 반지름에 따른 사분원 및 두 번째 직선의 길이를 합한다.
float GetPathLength()
{
	return firstLength + PI * 0.5f * cornerRadius + secondLength;
}

// 이동 거리가 속한 경로 구간에 따라 위치와 바라보는 y축 각도를 계산한다.
PathPose GetPathPose(float distance)
{
	// 첫 직선에서는 x를 고정하고 z를 감소시키며 이동한다.
	if (distance <= firstLength)
		return {{startX, beltHeight, startZ - distance}, 0.0f};
	// 90도 코너의 원호 길이를 반지름과 라디안 각도로 구한다.
	float arcLength = PI * 0.5f * cornerRadius;
	float centerX = startX + cornerRadius;
	float centerZ = startZ - firstLength;
	// 코너에서는 이동 거리를 각도로 바꾸어 위치와 방향을 함께 회전시킨다.
	if (distance < firstLength + arcLength)
	{
		float angle = (distance - firstLength) / cornerRadius;
		return {{centerX - cornerRadius * std::cos(angle), beltHeight,
			centerZ - cornerRadius * std::sin(angle)}, -glm::degrees(angle)};
	}
	// 코너 이후에는 z를 고정하고 x가 증가하는 두 번째 직선을 따라간다.
	return {{centerX + distance - firstLength - arcLength, beltHeight, centerZ - cornerRadius}, -90.0f};
}

// 경로의 위치로 이동하고 경로 방향으로 회전하는 부모 모델 행렬을 만든다.
glm::mat4 GetPathTransform(float distance)
{
	PathPose pose = GetPathPose(distance);
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), pose.position);
	return glm::rotate(transform, glm::radians(pose.angleY), glm::vec3(0.0f, 1.0f, 0.0f));
}

// 바닥 평면을 삼각형 두 개로 만들고 VAO와 VBO에 저장한다.
void InitFloorBuffer()
{
	std::vector<Vertex> vertices = {
		{-5.5f, 0, -5.5f, 1, 1, 1}, {5.5f, 0, -5.5f, 1, 1, 1}, {5.5f, 0, 5.5f, 1, 1, 1},
		{-5.5f, 0, -5.5f, 1, 1, 1}, {5.5f, 0, 5.5f, 1, 1, 1}, {-5.5f, 0, 5.5f, 1, 1, 1}
	};
	InitMeshBuffer(floorMesh, vertices);
}

// 여러 부품이 재사용할 정육면체 정점과 색상을 VAO와 VBO에 저장한다.
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

// 같은 경로를 따라 양쪽 가장자리를 연결하여 컨베이어 윗면을 삼각형으로 만든다.
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

// 벨트 양옆 난간과 코너 난간 및 바닥 지지대를 정적인 부품 목록으로 만든다.
void InitConveyorParts()
{
	conveyorParts.clear();
	// 고정 부품의 위치와 방향 및 크기를 최종 모델 행렬로 묶어 저장한다.
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
	// 경로 양옆에 같은 방법으로 난간을 배치한다.
	for (float side : {-1.0f, 1.0f})
	{
		float offset = side * (beltHalfWidth + 0.035f);
		add({startX + offset, beltHeight + 0.035f, startZ - (firstLength - padding) * 0.5f},
			{0.06f, 0.10f, firstLength + padding}, 0.0f, railColor);
		PathPose end = GetPathPose(cornerEnd + secondLength * 0.5f + padding * 0.5f);
		add({end.position.x, beltHeight + 0.035f, end.position.z + offset},
			{secondLength + padding, 0.10f, 0.06f}, 0.0f, railColor);
		// 코너 난간을 짧은 직선 조각으로 나누고 각 조각의 진행 방향을 계산한다.
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
	// 벨트 경로를 따라 여러 위치에 바닥 지지대를 배치한다.
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

// 실습에서 사용할 정점과 GPU 버퍼를 처음 한 번 준비한다.
void InitBuffer()
{
	InitFloorBuffer();
	InitCubeBuffer();
	InitBeltBuffer();
	InitConveyorParts();
}

// 벨트와 카메라를 초기화하고 번호와 색상이 다른 상자 다섯 개를 간격을 두어 배치한다.
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
		// 상자마다 시작 거리를 다르게 지정하여 서로 간격을 유지하게 한다.
		parcels[i] = {i + 1, 0.35f + GetPathLength() * i / 5.0f, {0.58f, 0.5f, 0.68f}, colors[i]};
}

// 방향키로 카메라를 공전시키고 더하기와 빼기 키로 줌을 조절한다.
void ProcessInput(float deltaTime)
{
	// 좌우 방향키를 누르는 동안 카메라를 장면 주위로 회전시킨다.
	if (glfwGetKey(mainWindow, GLFW_KEY_LEFT) == GLFW_PRESS) camera.orbitAngleY -= 50.0f * deltaTime;
	if (glfwGetKey(mainWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.orbitAngleY += 50.0f * deltaTime;
	// 일반 더하기와 빼기 및 숫자패드 입력으로 카메라 거리를 조절한다.
	bool zoomIn = glfwGetKey(mainWindow, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_KP_ADD) == GLFW_PRESS;
	bool zoomOut = glfwGetKey(mainWindow, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;
	if (zoomIn) camera.distance -= 4.0f * deltaTime;
	if (zoomOut) camera.distance += 4.0f * deltaTime;
	// 카메라가 장면에 지나치게 가까워지거나 멀어지지 않게 제한한다.
	camera.distance = std::clamp(camera.distance, 5.5f, 20.0f);
	camera.orbitAngleY = std::fmod(camera.orbitAngleY, 360.0f);
}

// 경과 시간에 따라 벨트의 공통 이동 거리를 증가시키고 끝에서 처음으로 순환시킨다.
void Update(float deltaTime)
{
	// 긴 프레임에서 이동이 과도하게 건너뛰지 않도록 경과 시간을 제한한다.
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	ProcessInput(deltaTime);
	// 이동 중에만 공통 거리를 늘리고 전체 경로 길이의 나머지로 순환시킨다.
	if (beltMoving) beltDistance = std::fmod(beltDistance + beltSpeed * deltaTime, GetPathLength());
}

// 전달받은 모델 행렬과 색상으로 공통 육면체 버퍼를 그린다.
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(transform));
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
	DrawMesh(cubeMesh);
}

// 부모 행렬에 부품의 위치와 크기를 결합해 줄무늬나 지지대 등의 육면체를 그린다.
void DrawPartScene(const glm::mat4& parent, const glm::vec3& center, const glm::vec3& size, const glm::vec3& color)
{
	DrawCubeScene(glm::scale(glm::translate(parent, center), size), color);
}

// 고정된 벨트와 프레임 및 상자와 같은 속도로 이동하는 줄무늬를 그린다.
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

// 일곱 막대의 비트 조합으로 숫자를 만들고 상자 표면에 부착한다.
void DrawNumberScene(int number, const glm::mat4& faceTransform)
{
	// 각 비트는 숫자를 구성하는 일곱 막대 중 하나를 뜻한다.
	// 숫자마다 켜야 하는 일곱 막대를 비트 단위로 지정한다.
	const unsigned int masks[6] = {0, 0x06, 0x5B, 0x4F, 0x66, 0x6D};
	const glm::vec3 centers[7] = {
		{0, 0.15f, 0.012f}, {0.09f, 0.075f, 0.012f}, {0.09f, -0.075f, 0.012f},
		{0, -0.15f, 0.012f}, {-0.09f, -0.075f, 0.012f}, {-0.09f, 0.075f, 0.012f}, {0, 0, 0.012f}
	};
	DrawPartScene(faceTransform, glm::vec3(0), {0.27f, 0.37f, 0.012f}, {0.95f, 0.95f, 0.85f});
	for (int i = 0; i < 7; ++i)
	{
		// 현재 숫자에 포함되지 않는 막대는 그리지 않는다.
		if ((masks[number] & (1u << i)) == 0) continue;
		bool horizontal = i == 0 || i == 3 || i == 6;
		glm::vec3 size = horizontal ? glm::vec3(0.18f, 0.025f, 0.012f) : glm::vec3(0.025f, 0.13f, 0.012f);
		DrawPartScene(faceTransform, centers[i], size, {0.04f, 0.04f, 0.04f});
	}
}

// 상자별 시작 거리와 공통 이동 거리로 위치와 방향을 구하고 번호까지 함께 그린다.
void DrawParcelScene(const Parcel& parcel)
{
	// 상자별 간격을 유지하면서 끝에 도달한 상자를 시작 위치로 돌려보낸다.
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

// 고정된 높이의 카메라 시점과 선택한 투영을 적용해 바닥과 컨베이어 및 상자를 그린다.
void DrawScene()
{
	glClearColor(0.5f, 0.72f, 0.9f, 1.0f);
	// 색상과 깊이 버퍼를 지워 이전 프레임의 결과를 제거한다.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 이후 그리기에 사용할 셰이더 프로그램을 활성화한다.
	glUseProgram(shaderProgram);
	float angle = glm::radians(camera.orbitAngleY);
	float elevation = glm::radians(camera.elevation);
	// 고정된 높이 각도와 공전 각도 및 거리로 카메라 위치를 구한다.
	glm::vec3 cameraPosition = camera.target + camera.distance * glm::vec3(
		std::cos(elevation) * std::sin(angle), std::sin(elevation), std::cos(elevation) * std::cos(angle));
	glm::mat4 view = glm::lookAt(cameraPosition, camera.target, glm::vec3(0, 1, 0));
	float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
	glm::mat4 projection;
	// 카메라 모드에 따라 원근 투영 또는 직각 투영을 적용한다.
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

// 벨트 이동 토글과 투영 선택 및 초기화와 종료 입력을 처리한다.
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action != GLFW_PRESS) return;
	// 벨트 줄무늬와 상자의 이동을 함께 시작하거나 멈춘다.
	if (key == GLFW_KEY_B) beltMoving = !beltMoving;
	// 소문자 p는 원근 투영이고 Shift와 p는 직각 투영이다.
	if (key == GLFW_KEY_P) camera.perspective = (mods & GLFW_MOD_SHIFT) == 0;
	// 상자 배치와 이동 거리 및 카메라 설정을 초기화한다.
	if (key == GLFW_KEY_R) ResetScene();
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
	DeleteMesh(floorMesh);
	DeleteMesh(cubeMesh);
	DeleteMesh(beltMesh);
	glDeleteProgram(shaderProgram);
}
