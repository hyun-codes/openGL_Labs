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

struct TankPart
{
	glm::mat4 transform;
	glm::vec3 color;
};

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

int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
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
	mainWindow = window;
	glfwSwapInterval(1);
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
	ResetScene();
	std::cout << "Arrows: move tank | d: lower height | t: central rotation | l: swap upper bodies\n"
		<< "g: opposite barrel rotation | p: opposite flag rotation\n"
		<< "Hold x/z: move camera (Shift reverses direction)\n"
		<< "y/Y: camera local rotation toggle | r/R: camera orbit toggle\n"
		<< "o: random colors | c: reset | q: quit" << std::endl;

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

void ProcessInput(float deltaTime)
{
	glm::vec3 movement(0.0f);
	if (glfwGetKey(mainWindow, GLFW_KEY_LEFT) == GLFW_PRESS) movement.x -= 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) movement.x += 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_UP) == GLFW_PRESS) movement.z -= 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_DOWN) == GLFW_PRESS) movement.z += 1.0f;
	if (glm::length(movement) > 0.0f) tank.position += glm::normalize(movement) * moveSpeed * deltaTime;

	bool shift = glfwGetKey(mainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(mainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	float direction = shift ? -1.0f : 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_X) == GLFW_PRESS) camera.position.x += direction * 2.0f * deltaTime;
	if (glfwGetKey(mainWindow, GLFW_KEY_Z) == GLFW_PRESS) camera.position.z += direction * 2.0f * deltaTime;
}

void AddTankPart(std::vector<TankPart>& parts, const glm::mat4& parent,
	const glm::vec3& center, const glm::vec3& size, int colorIndex)
{
	glm::mat4 transform = glm::translate(parent, center);
	transform = glm::scale(transform, size);
	parts.push_back({transform, tankColors[colorIndex]});
}

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
	glm::mat4 centralTransform = glm::translate(tankTransform, glm::vec3(0.0f, tank.lowerHeight + centralHeight * 0.5f, 0.0f));
	centralTransform = glm::rotate(centralTransform, glm::radians(tank.centralRotateAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
	AddTankPart(parts, centralTransform, glm::vec3(0.0f), {1.85f, centralHeight, 2.0f}, 2);

	float swapAngle = glm::radians(tank.upperSwapAngleY);
	for (int i = 0; i < 2; ++i)
	{
		float direction = i == 0 ? -1.0f : 1.0f;
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

void KeepTankOnFloor()
{
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

void Update(float deltaTime)
{
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	ProcessInput(deltaTime);
	if (tank.isLowerScaling)
	{
		tank.lowerScaleAngle = std::fmod(tank.lowerScaleAngle + 90.0f * deltaTime, 360.0f);
		tank.lowerHeight = 0.45f + 0.15f * std::sin(glm::radians(tank.lowerScaleAngle));
	}
	if (tank.isCentralRotating) tank.centralRotateAngleY = std::fmod(tank.centralRotateAngleY + rotateSpeed * deltaTime, 360.0f);
	if (tank.isBarrelRotating) tank.barrelRotateAngleY = std::fmod(tank.barrelRotateAngleY + rotateSpeed * deltaTime, 360.0f);
	if (tank.isFlagRotating) tank.flagRotateAngleX = std::fmod(tank.flagRotateAngleX + rotateSpeed * deltaTime, 360.0f);
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

void DrawFloorScene(const glm::mat4& view, const glm::mat4& projection)
{
	SetTransformUniforms(shaderProgram, glm::mat4(1.0f), view, projection);
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(floorColor));
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
	glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.direction, camera.up);
	glm::mat4 projection = glm::perspective(glm::radians(camera.fovDegrees), aspect, camera.nearPlane, camera.farPlane);

	DrawFloorScene(view, projection);
	DrawTankScene(view, projection);
}

void DrawTankScene(const glm::mat4& view, const glm::mat4& projection)
{
	for (const TankPart& part : BuildTankParts())
		DrawCubeScene(part.transform, part.color, view, projection);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	if (action != GLFW_PRESS) return;
	if (key == GLFW_KEY_D) tank.isLowerScaling = !tank.isLowerScaling;
	if (key == GLFW_KEY_T) tank.isCentralRotating = !tank.isCentralRotating;
	if (key == GLFW_KEY_L) tank.upperSwapTargetAngleY += 180.0f;
	if (key == GLFW_KEY_G) tank.isBarrelRotating = !tank.isBarrelRotating;
	if (key == GLFW_KEY_P) tank.isFlagRotating = !tank.isFlagRotating;
	if (key == GLFW_KEY_Y || key == GLFW_KEY_R)
	{
		float velocity = (mods & GLFW_MOD_SHIFT) ? rotateSpeed : -rotateSpeed;
		float& currentVelocity = key == GLFW_KEY_Y ? camera.rotateVelocityY : camera.orbitVelocityY;
		currentVelocity = currentVelocity == velocity ? 0.0f : velocity;
	}
	if (key == GLFW_KEY_O) RandomizeColors();
	if (key == GLFW_KEY_C) ResetScene();
	if (key == GLFW_KEY_Q) glfwSetWindowShouldClose(window, GLFW_TRUE);
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
