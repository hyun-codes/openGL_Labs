#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <vector>

// 카메라 위치와 시선 및 자전과 공전 속도를 관리한다.
struct Camera
{
	glm::vec3 position{4.0f, 3.0f, -6.0f};
	glm::vec3 direction{0.0f, 0.0f, -1.0f};
	glm::vec3 up{0.0f, 1.0f, 0.0f};
	float rotateVelocityY = 0.0f;
	float orbitVelocityY = 0.0f;
	float fovDegrees = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
};

// 탱크 위치와 각 부품의 애니메이션 상태 및 토글을 저장한다.
struct Tank
{
	glm::vec3 position{0.0f};
	float lowerHeight = 0.45f;
	float lowerScaleAngle = 0.0f;
	float centralRotateAngleY = 0.0f;
	float upperSwapAngleY = 0.0f;
	float upperSwapTargetAngleY = 0.0f;
	float barrelRotateAngleY = 0.0f;
	float flagRotateAngleX = 0.0f;
	bool isLowerScaling = false;
	bool isCentralRotating = false;
	bool isBarrelRotating = false;
	bool isFlagRotating = false;
};

// 부품별 최종 모델 행렬과 색상 번호를 한 묶음으로 저장한다.
struct TankPart
{
	glm::mat4 transform;
	glm::vec3 color;
};

// 셰이더 프로그램과 재사용할 GPU 메시 및 장면 상태를 관리한다.
GLuint shaderProgram = 0;
Mesh floorMesh;
Mesh cubeMesh;
Camera camera;
Tank tank;
GLFWwindow* mainWindow = nullptr;
int framebufferWidth = 800;
int framebufferHeight = 800;
const float floorHalfSize = 4.0f;
const float moveSpeed = 1.4f;
const float rotateSpeed = 60.0f;

const std::array<glm::vec3, 11> initialTankColors = {{
	{0.3f, 0.34f, 0.4f}, {0.45f, 0.4f, 0.35f}, {0.25f, 0.55f, 0.8f},
	{0.35f, 0.8f, 0.25f}, {0.95f, 0.6f, 0.15f}, {0.95f, 0.85f, 0.15f}, {0.95f, 0.25f, 0.25f},
	{0.55f, 0.3f, 0.85f}, {0.2f, 0.85f, 0.85f}, {0.9f, 0.65f, 0.1f}, {0.3f, 0.45f, 1.0f}
}};
std::array<glm::vec3, 11> tankColors = initialTankColors;
glm::vec3 floorColor(0.65f, 0.78f, 0.86f);

void InitFloorBuffer();
void InitCubeBuffer();
void InitBuffer();
void ResetScene();
void RandomizeColors();
void ProcessInput(float deltaTime);
void KeepTankOnFloor();
std::vector<TankPart> BuildTankParts();
void AddTankPart(std::vector<TankPart>& parts, const glm::mat4& parent,
	const glm::vec3& center, const glm::vec3& size, int colorIndex);
void Update(float deltaTime);
void DrawScene();
void DrawTankScene(const glm::mat4& view, const glm::mat4& projection);
void DrawFloorScene(const glm::mat4& view, const glm::mat4& projection);
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color,
	const glm::mat4& view, const glm::mat4& projection);
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
	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice19", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	// 현재 스레드에서 사용할 OpenGL 컨텍스트를 선택한다.
	glfwMakeContextCurrent(window);
	mainWindow = window;
	glfwSwapInterval(1);
	// 키보드 입력이 발생할 때 호출할 함수를 등록한다.
	glfwSetKeyCallback(window, keyCallBack);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);

	// 컨텍스트 생성 뒤 GLEW를 초기화하여 OpenGL 함수 주소를 불러온다.
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	glViewport(0, 0, framebufferWidth, framebufferHeight);
	// 깊이 버퍼를 사용하여 가까운 표면이 먼 표면을 가리도록 한다.
	glEnable(GL_DEPTH_TEST);

	// GLSL 파일을 읽어 컴파일하고 실행할 셰이더 프로그램으로 연결한다.
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (shaderProgram == 0)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	InitBuffer();
	ResetScene();
	std::cout << "Arrows: move tank | d: lower height | t: central rotation | l: swap upper bodies\n"
		<< "g: opposite barrel rotation | p: opposite flag rotation\n"
		<< "Hold x/z: move camera (Shift reverses direction)\n"
		<< "y/Y: camera local rotation toggle | r/R: camera orbit toggle\n"
		<< "o: random colors | c: reset | q: quit" << std::endl;

	double lastFrameTime = glfwGetTime();
	// 종료 요청 전까지 상태 갱신과 렌더링 및 이벤트 처리를 반복한다.
	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - lastFrameTime);
		lastFrameTime = currentTime;
		Update(deltaTime);
		DrawScene();
		// 완성된 뒤쪽 버퍼를 화면에 표시한다.
		glfwSwapBuffers(window);
		// 대기 중인 키보드와 마우스 및 창 이벤트를 처리한다.
		glfwPollEvents();
	}
	Cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

// 바닥 평면을 삼각형 두 개로 만들고 VAO와 VBO에 저장한다.
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

// 여러 부품이 재사용할 정육면체 정점과 색상을 VAO와 VBO에 저장한다.
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

// 실습에서 사용할 정점과 GPU 버퍼를 처음 한 번 준비한다.
void InitBuffer()
{
	InitFloorBuffer();
	InitCubeBuffer();
}

// 객체와 카메라 및 애니메이션 상태를 처음 값으로 되돌린다.
void ResetScene()
{
	tank = Tank{};
	camera = Camera{};
	camera.direction = glm::normalize(glm::vec3(0.0f, 0.7f, 0.0f) - camera.position);
	glm::vec3 right = glm::normalize(glm::cross(camera.direction, camera.up));
	camera.up = glm::normalize(glm::cross(right, camera.direction));
	tankColors = initialTankColors;
	floorColor = glm::vec3(0.65f, 0.78f, 0.86f);
}

// 탱크의 각 부품과 바닥에 적용할 색상을 무작위로 다시 정한다.
void RandomizeColors()
{
	for (glm::vec3& color : tankColors)
	{
		color.r = 0.2f + 0.75f * static_cast<float>(std::rand()) / RAND_MAX;
		color.g = 0.2f + 0.75f * static_cast<float>(std::rand()) / RAND_MAX;
		color.b = 0.2f + 0.75f * static_cast<float>(std::rand()) / RAND_MAX;
	}
	floorColor = glm::vec3(0.55f + 0.3f * static_cast<float>(std::rand()) / RAND_MAX,
		0.55f + 0.3f * static_cast<float>(std::rand()) / RAND_MAX,
		0.55f + 0.3f * static_cast<float>(std::rand()) / RAND_MAX);
}

// 누르고 있는 키를 매 프레임 확인하고 경과 시간에 비례하여 이동한다.
void ProcessInput(float deltaTime)
{
	// 방향키의 입력을 합쳐 탱크의 이동 방향을 만든다.
	glm::vec3 movement(0.0f);
	if (glfwGetKey(mainWindow, GLFW_KEY_LEFT) == GLFW_PRESS) movement.x -= 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) movement.x += 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_UP) == GLFW_PRESS) movement.z -= 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_DOWN) == GLFW_PRESS) movement.z += 1.0f;
	if (glm::length(movement) > 0.0f) tank.position += glm::normalize(movement) * moveSpeed * deltaTime;

	// Shift 여부로 카메라의 x축과 z축 이동 방향을 반전한다.
	bool shift = glfwGetKey(mainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(mainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	float direction = shift ? -1.0f : 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_X) == GLFW_PRESS) camera.position.x += direction * 2.0f * deltaTime;
	if (glfwGetKey(mainWindow, GLFW_KEY_Z) == GLFW_PRESS) camera.position.z += direction * 2.0f * deltaTime;
}

// 부모 변환에 부품의 위치와 크기를 결합하여 탱크 부품 목록에 저장한다.
void AddTankPart(std::vector<TankPart>& parts, const glm::mat4& parent,
	const glm::vec3& center, const glm::vec3& size, int colorIndex)
{
	glm::mat4 transform = glm::translate(parent, center);
	transform = glm::scale(transform, size);
	parts.push_back({transform, tankColors[colorIndex]});
}

// 아래 몸체부터 포신과 깃대까지 부모와 자식 변환으로 탱크 전체를 조립한다.
std::vector<TankPart> BuildTankParts()
{
	std::vector<TankPart> parts;
	parts.reserve(11);
	glm::mat4 tankTransform = glm::translate(glm::mat4(1.0f), tank.position);

	// 아래 몸체는 바닥에 붙이고, 높이가 바뀌면 나머지 몸체도 그 위로 이동한다.
	AddTankPart(parts, tankTransform, {-0.62f, tank.lowerHeight * 0.5f, 0.0f},
		{0.42f, tank.lowerHeight, 2.2f}, 0);
	AddTankPart(parts, tankTransform, {0.62f, tank.lowerHeight * 0.5f, 0.0f},
		{0.5f, tank.lowerHeight, 2.05f}, 1);

	const float centralHeight = 0.24f;
	// 아래 몸체 위에 중앙 몸체를 배치하고 중앙 몸체만 자전시킨다.
	glm::mat4 centralTransform = glm::translate(tankTransform, glm::vec3(0.0f, tank.lowerHeight + centralHeight * 0.5f, 0.0f));
	centralTransform = glm::rotate(centralTransform, glm::radians(tank.centralRotateAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
	AddTankPart(parts, centralTransform, glm::vec3(0.0f), {1.85f, centralHeight, 2.0f}, 2);

	// 두 상부 몸체를 중앙 몸체의 자식으로 배치하고 교환 각도를 적용한다.
	float swapAngle = glm::radians(tank.upperSwapAngleY);
	for (int i = 0; i < 2; ++i)
	{
		float direction = i == 0 ? -1.0f : 1.0f;
		// 반대쪽 상부 몸체가 서로 마주 보는 경로로 이동하도록 위치를 계산한다.
		glm::vec3 upperPosition(direction * 0.48f * std::cos(swapAngle), centralHeight * 0.5f,
			direction * 0.65f * std::sin(swapAngle));
		glm::mat4 upperTransform = glm::translate(centralTransform, upperPosition);
		float upperHeight = i == 0 ? 0.36f : 0.3f;
		float upperDepth = i == 0 ? 0.7f : 0.58f;
		int colorIndex = 3 + i * 4;
		AddTankPart(parts, upperTransform, {0.0f, upperHeight * 0.5f, 0.0f},
			{i == 0 ? 0.6f : 0.5f, upperHeight, upperDepth}, colorIndex);

		// 포신은 상부 몸체 앞쪽의 연결 지점을 중심으로 서로 반대로 회전한다.
		glm::mat4 barrelTransform = glm::translate(upperTransform, glm::vec3(0.0f, upperHeight * 0.55f, -upperDepth * 0.5f));
		barrelTransform = glm::rotate(barrelTransform, glm::radians(tank.barrelRotateAngleY * direction), glm::vec3(0.0f, 1.0f, 0.0f));
		float barrelLength = i == 0 ? 0.65f : 0.75f;
		AddTankPart(parts, barrelTransform, {0.0f, 0.0f, -barrelLength * 0.5f},
			{0.075f, 0.075f, barrelLength}, colorIndex + 1);

		// 깃대와 깃발은 같은 밑부분을 중심으로 움직인다.
		glm::mat4 flagTransform = glm::translate(upperTransform, glm::vec3(0.0f, upperHeight, 0.0f));
		flagTransform = glm::rotate(flagTransform, glm::radians(tank.flagRotateAngleX * direction), glm::vec3(1.0f, 0.0f, 0.0f));
		float poleHeight = i == 0 ? 0.62f : 0.52f;
		AddTankPart(parts, flagTransform, {0.0f, poleHeight * 0.5f, 0.0f}, {0.05f, poleHeight, 0.05f}, colorIndex + 2);
		AddTankPart(parts, flagTransform, {0.14f, poleHeight - 0.1f, 0.0f}, {0.28f, 0.18f, 0.035f}, colorIndex + 3);
	}
	return parts;
}

// 모든 부품의 변환된 꼭짓점을 검사하고 탱크가 바닥 영역을 벗어나지 않게 보정한다.
void KeepTankOnFloor()
{
	// 회전과 크기 변환까지 적용된 탱크 전체의 최소와 최대 범위를 구한다.
	glm::vec3 minimum(std::numeric_limits<float>::max());
	glm::vec3 maximum(std::numeric_limits<float>::lowest());
	for (const TankPart& part : BuildTankParts())
	{
		for (int x : {-1, 1})
			for (int y : {-1, 1})
				for (int z : {-1, 1})
				{
					glm::vec3 point = glm::vec3(part.transform * glm::vec4(x * 0.5f, y * 0.5f, z * 0.5f, 1.0f));
					minimum = glm::min(minimum, point);
					maximum = glm::max(maximum, point);
				}
	}
	// 회전 중인 포신과 깃대까지 포함해 바닥의 xz 경계 안에 둔다.
	if (minimum.x < -floorHalfSize) tank.position.x += -floorHalfSize - minimum.x;
	if (maximum.x > floorHalfSize) tank.position.x -= maximum.x - floorHalfSize;
	if (minimum.z < -floorHalfSize) tank.position.z += -floorHalfSize - minimum.z;
	if (maximum.z > floorHalfSize) tank.position.z -= maximum.z - floorHalfSize;
}

// 키 입력과 탱크 부품 애니메이션 및 카메라 자전과 공전을 한 프레임 갱신한다.
void Update(float deltaTime)
{
	// 긴 프레임에서 이동이 과도하게 건너뛰지 않도록 경과 시간을 제한한다.
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	ProcessInput(deltaTime);
	// 아래 몸체의 높이를 주기적으로 바꾸고 다른 부품이 그 위를 따라가게 한다.
	if (tank.isLowerScaling)
	{
		tank.lowerScaleAngle = std::fmod(tank.lowerScaleAngle + 90.0f * deltaTime, 360.0f);
		tank.lowerHeight = 0.45f + 0.15f * std::sin(glm::radians(tank.lowerScaleAngle));
	}
	// 중앙 몸체와 그 자식 부품이 공유할 회전 각도를 갱신한다.
	if (tank.isCentralRotating) tank.centralRotateAngleY = std::fmod(tank.centralRotateAngleY + rotateSpeed * deltaTime, 360.0f);
	// 좌우 포신이 반대 방향으로 돌 때 사용할 공통 각도를 증가시킨다.
	if (tank.isBarrelRotating) tank.barrelRotateAngleY = std::fmod(tank.barrelRotateAngleY + rotateSpeed * deltaTime, 360.0f);
	// 좌우 깃대가 반대 방향으로 돌 때 사용할 공통 각도를 증가시킨다.
	if (tank.isFlagRotating) tank.flagRotateAngleX = std::fmod(tank.flagRotateAngleX + rotateSpeed * deltaTime, 360.0f);
	// 상부 몸체의 교환을 목표 각도까지 부드럽게 진행한다.
	if (tank.upperSwapAngleY < tank.upperSwapTargetAngleY)
	{
		tank.upperSwapAngleY = std::min(tank.upperSwapAngleY + 90.0f * deltaTime, tank.upperSwapTargetAngleY);
		if (tank.upperSwapAngleY == tank.upperSwapTargetAngleY && tank.upperSwapAngleY >= 360.0f)
		{
			float completedTurns = std::floor(tank.upperSwapAngleY / 360.0f) * 360.0f;
			tank.upperSwapAngleY -= completedTurns;
			tank.upperSwapTargetAngleY -= completedTurns;
		}
	}
	KeepTankOnFloor();

	// 카메라 자전: 위치는 유지하고 자신의 위쪽 축을 기준으로 시선만 회전한다.
	glm::mat4 cameraRotation = glm::rotate(glm::mat4(1.0f),
		glm::radians(camera.rotateVelocityY * deltaTime), camera.up);
	camera.direction = glm::normalize(glm::vec3(cameraRotation * glm::vec4(camera.direction, 0.0f)));

	// 카메라 공전: 원점을 지나는 y축 주위로 위치와 시선 방향을 함께 회전한다.
	glm::mat4 cameraOrbit = glm::rotate(glm::mat4(1.0f),
		glm::radians(camera.orbitVelocityY * deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
	camera.position = glm::vec3(cameraOrbit * glm::vec4(camera.position, 1.0f));
	camera.direction = glm::normalize(glm::vec3(cameraOrbit * glm::vec4(camera.direction, 0.0f)));
	camera.up = glm::normalize(glm::vec3(cameraOrbit * glm::vec4(camera.up, 0.0f)));
}

// 해당 화면의 카메라와 투영 행렬을 적용하여 바닥을 그린다.
void DrawFloorScene(const glm::mat4& view, const glm::mat4& projection)
{
	SetTransformUniforms(shaderProgram, glm::mat4(1.0f), view, projection);
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(floorColor));
	DrawMesh(floorMesh);
}

// 한 부품의 모델 행렬과 색상 및 카메라 행렬을 전달하고 육면체를 그린다.
void DrawCubeScene(const glm::mat4& transform, const glm::vec3& color,
	const glm::mat4& view, const glm::mat4& projection)
{
	SetTransformUniforms(shaderProgram, transform, view, projection);
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
	DrawMesh(cubeMesh);
}

// 카메라의 위치와 시선으로 뷰 행렬을 만들고 원근 투영으로 바닥과 탱크를 그린다.
void DrawScene()
{
	glClearColor(0.75f, 0.85f, 0.95f, 1.0f);
	// 색상과 깊이 버퍼를 지워 이전 프레임의 결과를 제거한다.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 이후 그리기에 사용할 셰이더 프로그램을 활성화한다.
	glUseProgram(shaderProgram);
	float aspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
	glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.direction, camera.up);
	glm::mat4 projection = glm::perspective(glm::radians(camera.fovDegrees), aspect, camera.nearPlane, camera.farPlane);

	DrawFloorScene(view, projection);
	DrawTankScene(view, projection);
}

// 같은 탱크 부품 목록을 순회하며 각 부품을 해당 카메라에서 그린다.
void DrawTankScene(const glm::mat4& view, const glm::mat4& projection)
{
	for (const TankPart& part : BuildTankParts())
		DrawCubeScene(part.transform, part.color, view, projection);
}

// 키를 처음 누를 때 탱크 애니메이션과 카메라 회전 및 초기화 명령을 처리한다.
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action != GLFW_PRESS) return;
	// 아래 몸체의 높이 확대와 축소 애니메이션을 시작하거나 멈춘다.
	if (key == GLFW_KEY_D) tank.isLowerScaling = !tank.isLowerScaling;
	// 중앙 몸체의 y축 회전 애니메이션을 시작하거나 멈춘다.
	if (key == GLFW_KEY_T) tank.isCentralRotating = !tank.isCentralRotating;
	// 상부 몸체가 다음 180도 교환 위치까지 이동하도록 목표를 추가한다.
	if (key == GLFW_KEY_L) tank.upperSwapTargetAngleY += 180.0f;
	// 포신 두 개의 반대 방향 y축 회전을 시작하거나 멈춘다.
	if (key == GLFW_KEY_G) tank.isBarrelRotating = !tank.isBarrelRotating;
	// 깃대 두 개의 반대 방향 x축 회전을 시작하거나 멈춘다.
	if (key == GLFW_KEY_P) tank.isFlagRotating = !tank.isFlagRotating;
	// 제자리 회전과 공전을 구분하고 Shift로 방향을 반전하며 같은 입력으로 정지한다.
	if (key == GLFW_KEY_Y || key == GLFW_KEY_R)
	{
		float velocity = (mods & GLFW_MOD_SHIFT) ? rotateSpeed : -rotateSpeed;
		float& currentVelocity = key == GLFW_KEY_Y ? camera.rotateVelocityY : camera.orbitVelocityY;
		currentVelocity = currentVelocity == velocity ? 0.0f : velocity;
	}
	// 탱크의 부품과 바닥 색상을 다시 무작위로 정한다.
	if (key == GLFW_KEY_O) RandomizeColors();
	// 탱크와 카메라의 모든 상태를 처음 값으로 되돌린다.
	if (key == GLFW_KEY_C) ResetScene();
	// 윈도우 종료 플래그를 설정하여 메인 반복문을 끝낸다.
	if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// 실제 화면 크기를 저장하고 뷰포트를 갱신해 창 크기 변경에 대응한다.
void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	framebufferWidth = width > 0 ? width : 1;
	framebufferHeight = height > 0 ? height : 1;
	glViewport(0, 0, width, height);
}

// OpenGL 컨텍스트가 살아 있는 동안 생성한 버퍼와 셰이더 프로그램을 해제한다.
void Cleanup()
{
	DeleteMesh(floorMesh);
	DeleteMesh(cubeMesh);
	glDeleteProgram(shaderProgram);
}
