#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

struct Camera
{
	glm::vec3 position{0.0f, 3.2f, 9.0f};
	glm::vec3 target{ 0.0f, 0.0f, 0.0f };	//타겟은 원점.
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	float fovDegrees = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
};

struct Planet
{
	float orbitTilt;
	float orbitAngle;
	float orbitSpeed;
	float radius;
	glm::vec3 color;
	float moonAngle;
	float moonSpeed;
	float moonOrbitRadius;
	float moonRadius;
	glm::vec3 moonColor;
};

const Planet initialPlanets[3] = {
	{  0.0f,   0.0f,  22.0f, 0.18f, {0.1f, 0.9f, 0.35f},  30.0f,   85.0f, 0.38f, 0.065f, {1.0f, 0.15f, 0.15f}},
	{ 45.0f, 140.0f, -31.0f, 0.20f, {1.0f, 0.5f, 0.1f},  150.0f, -110.0f, 0.42f, 0.070f, {0.15f, 0.9f, 1.0f}},
	{-45.0f, 250.0f,  42.0f, 0.16f, {0.75f, 0.2f, 1.0f}, 270.0f,  135.0f, 0.35f, 0.055f, {1.0f, 0.95f, 0.1f}}
};
Planet planets[3] = {initialPlanets[0], initialPlanets[1], initialPlanets[2]};

GLuint shaderProgram = 0;

Mesh sphereMesh;
Mesh orbitMesh;
Camera camera;
GLFWwindow* mainWindow = nullptr;
glm::vec3 scenePosition(0.0f);

int framebufferWidth = 800;
int framebufferHeight = 800;

const float PI = 3.14159265359f;
float orbitRadius = 2.0f;

double lastFrameTime = 0.0f;

bool depthTestEnabled = true;
bool isWireFace = false;
bool isPerspective = true;
bool isOrbitScaling = false;
bool isCenterAnimating = false;
bool isMoonOrbitRotating = false;
float centerAnimationAngle = 0.0f;
float moonOrbitRotateAngle = 0.0f;

bool LoadObjVertices(const char* filePath, std::vector<Vertex>& vertices);
bool InitBuffer();
void InitOrbitBuffer();
void ResetScene();
void ProcessInput(float deltaTime);
void Update();
void DrawScene();
void DrawSphereScene(const glm::mat4& modelTransform, const glm::vec3& color);
void DrawOrbitScene(const glm::mat4& orbitTransform, const glm::vec3& color);
void Cleanup();
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice18", nullptr, nullptr);
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
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);
	glEnable(GL_DEPTH_TEST);

	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (shaderProgram == 0)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	if (!InitBuffer())
	{
		Cleanup();
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	ResetScene();
	std::cout << "p/P: orthographic/perspective | m/M: solid/wireframe\n"
		<< "Hold W/A/S/D: move scene | +/-: move scene along z\n"
		<< "1: enlarged/original orbit radii | 2: center size/color | 3: moon orbit rotation\n"
		<< "H: depth test | R: reset | Q: quit" << std::endl;
	while (!glfwWindowShouldClose(window))
	{
		Update();
		DrawScene();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	Cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

bool LoadObjVertices(const char* filePath, std::vector<Vertex>& vertices)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "OBJ 파일을 열 수 없습니다: " << filePath << std::endl;
		return false;
	}

	std::vector<glm::vec3> positions;
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream stream(line);
		std::string type;
		stream >> type;

		if (type == "v")
		{
			glm::vec3 position;
			if (stream >> position.x >> position.y >> position.z)
				positions.push_back(position);
		}
		else if (type == "f")
		{
			std::vector<glm::vec3> face;
			std::string token;
			while (stream >> token)
			{
				// f의 v/vt/vn 중 위치 인덱스(v)만 읽는다.
				int index = 0;
				try { index = std::stoi(token.substr(0, token.find('/'))); }
				catch (...) { std::cerr << "잘못된 OBJ 면 인덱스: " << token << std::endl; return false; }
				if (index <= 0 || static_cast<size_t>(index) > positions.size())
				{
					std::cerr << "OBJ 면 인덱스 범위 오류: " << index << std::endl;
					return false;
				}
				face.push_back(positions[static_cast<size_t>(index - 1)]);
			}

			// 면을 삼각형으로 나눈다. 현재 sphere.obj의 면은 모두 삼각형이다.
			for (size_t i = 1; i + 1 < face.size(); ++i)
			{
				for (const glm::vec3& p : {face[0], face[i], face[i + 1]})
				{
					// 앞면과 뒷면의 밝기를 달리해 깊이 테스트 결과를 확인한다.
					float brightness = 0.2f + 0.8f * (p.z + 1.0f) * 0.5f;
					vertices.push_back({p.x, p.y, p.z, brightness, brightness, brightness});
				}
			}
		}
	}

	if (vertices.empty())
	{
		std::cerr << "OBJ에 삼각형이 없습니다: " << filePath << std::endl;
		return false;
	}
	return true;
}

bool InitBuffer()
{
	std::vector<Vertex> sphereVertices;
	if (!LoadObjVertices("models/sphere.obj", sphereVertices)) 
		return false;
	InitMeshBuffer(sphereMesh, sphereVertices);
	if (sphereMesh.VAO == 0) 
		return false;

	InitOrbitBuffer();

	return true;
}

void InitOrbitBuffer()
{
	std::vector<Vertex> orbitPosition;
	int segments = 96;						

	for (int i = 0; i < segments; ++i) {
		float theta = 2.0f * PI * float(i) / segments;
		// 반지름 1인 원을 만들고, 그릴 때 행성/달의 궤도 반지름을 적용한다.
		float x = std::cos(theta);
		float z = std::sin(theta);
		orbitPosition.push_back({ x, 0, z, 1.0, 1.0f, 1.0f });
	}	
	InitMeshBuffer(orbitMesh, orbitPosition, GL_LINE_LOOP);
}

void ResetScene()
{
	camera = Camera{};
	for (int i = 0; i < 3; ++i) planets[i] = initialPlanets[i];
	scenePosition = glm::vec3(0.0f);
	orbitRadius = 2.0f;
	isPerspective = true;
	isWireFace = false;
	depthTestEnabled = true;
	isOrbitScaling = isCenterAnimating = isMoonOrbitRotating = false;
	centerAnimationAngle = moonOrbitRotateAngle = 0.0f;
	glEnable(GL_DEPTH_TEST);
	lastFrameTime = glfwGetTime();
}

void ProcessInput(float deltaTime)
{
	glm::vec3 movement(0.0f);
	if (glfwGetKey(mainWindow, GLFW_KEY_W) == GLFW_PRESS) movement.y += 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_S) == GLFW_PRESS) movement.y -= 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_D) == GLFW_PRESS) movement.x += 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_KP_ADD) == GLFW_PRESS) movement.z += 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) movement.z -= 1.0f;
	if (glm::length(movement) > 0.0f) scenePosition += glm::normalize(movement) * 1.2f * deltaTime;
}

void Update()
{
	double currentTime = glfwGetTime();
	float deltaTime = static_cast<float>(currentTime - lastFrameTime);
	lastFrameTime = currentTime;
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	ProcessInput(deltaTime);
	for (Planet& planet : planets)
	{
		planet.orbitAngle = std::fmod(planet.orbitAngle + planet.orbitSpeed * deltaTime, 360.0f);
		planet.moonAngle = std::fmod(planet.moonAngle + planet.moonSpeed * deltaTime, 360.0f);
	}
	if (isCenterAnimating) centerAnimationAngle = std::fmod(centerAnimationAngle + 70.0f * deltaTime, 360.0f);
	if (isMoonOrbitRotating) moonOrbitRotateAngle = std::fmod(moonOrbitRotateAngle + 40.0f * deltaTime, 360.0f);
}

void DrawScene()
{
	glClearColor(0.025f, 0.025f, 0.07f, 1.0f);					//배경색.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);

	if (isWireFace)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;

	glm::mat4 view = glm::lookAt(camera.position, camera.target, camera.up);
	glm::mat4 projection;
	if (isPerspective)
		projection = glm::perspective(glm::radians(camera.fovDegrees), aspect, camera.nearPlane, camera.farPlane);
	else
	{
		float halfHeight = glm::length(camera.position - camera.target) * std::tan(glm::radians(camera.fovDegrees) * 0.5f);
		projection = glm::ortho(-halfHeight * aspect, halfHeight * aspect, -halfHeight, halfHeight, camera.nearPlane, camera.farPlane);
	}
	glm::mat4 sceneTransform = glm::translate(glm::mat4(1.0f), scenePosition);
	float orbitScale = isOrbitScaling ? 1.25f : 1.0f;
	float centerRadius = 0.45f + 0.15f * std::sin(glm::radians(centerAnimationAngle));
	float colorBlend = 0.5f - 0.5f * std::cos(glm::radians(centerAnimationAngle));
	glm::vec3 centerColor = glm::mix(glm::vec3(0, 0, 1), glm::vec3(1.0f, 0.25f, 0.3f), colorBlend);

	// 카메라와 투영은 모든 구와 궤도가 공유한다.
	SetTransformUniforms(shaderProgram, glm::mat4(1.0f), view, projection);
	DrawSphereScene(glm::scale(sceneTransform, glm::vec3(centerRadius)), centerColor);

	// 카메라의 회전만 역변환해 달의 궤도면을 화면과 평행하게 만든다.
	glm::mat4 moonPlane = glm::mat4(glm::transpose(glm::mat3(view)));
	moonPlane = glm::rotate(moonPlane, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// 달 궤도의 추가 회전은 행성 위치를 중심으로 z축에 적용한다.
	glm::mat4 moonOrbitRotation = glm::rotate(glm::mat4(1.0f), glm::radians(moonOrbitRotateAngle), glm::vec3(0, 0, 1));

	for (const Planet& planet : planets)
	{
		glm::mat4 orbitPlane = glm::rotate(
			glm::mat4(1.0f),
			glm::radians(planet.orbitTilt),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		float planetOrbitRadius = orbitRadius * orbitScale;
		DrawOrbitScene(glm::scale(sceneTransform * orbitPlane, glm::vec3(planetOrbitRadius)), glm::vec3(0.6f));

		// 궤도와 같은 평면·반지름으로 행성 위치를 계산한다.
		float angle = glm::radians(planet.orbitAngle);
		glm::vec3 orbitPosition(planetOrbitRadius * std::cos(angle), 0.0f, planetOrbitRadius * std::sin(angle));
		glm::vec3 planetPosition = glm::vec3(orbitPlane * glm::vec4(orbitPosition, 1.0f));
		glm::mat4 planetTransform = glm::translate(sceneTransform, planetPosition);
		DrawSphereScene(glm::scale(planetTransform, glm::vec3(planet.radius)), planet.color);

		// 행성의 크기 변환은 제외하고 위치만 달과 달 궤도에 전달한다.
		glm::mat4 moonFrame = planetTransform * moonOrbitRotation * moonPlane;
		float moonOrbitRadius = planet.moonOrbitRadius * orbitScale;
		DrawOrbitScene(glm::scale(moonFrame, glm::vec3(moonOrbitRadius)), glm::vec3(0.7f));

		float moonAngle = glm::radians(planet.moonAngle);
		glm::vec3 moonPosition(moonOrbitRadius * std::cos(moonAngle), 0.0f,
			moonOrbitRadius * std::sin(moonAngle));
		glm::mat4 moonTransform = glm::translate(moonFrame, moonPosition);
		DrawSphereScene(glm::scale(moonTransform, glm::vec3(planet.moonRadius)), planet.moonColor);
	}
}

void DrawSphereScene(const glm::mat4& modelTransform, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(modelTransform));
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
	DrawMesh(sphereMesh);
}

void DrawOrbitScene(const glm::mat4& orbitTransform, const glm::vec3& color)
{
	GLint modelLocation = glGetUniformLocation(shaderProgram, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(orbitTransform));

	GLint modelColor = glGetUniformLocation(shaderProgram, "objectColor");
	glUniform3fv(modelColor, 1, glm::value_ptr(color));
	DrawMesh(orbitMesh);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;

	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_H) {
		depthTestEnabled = !depthTestEnabled;

		if (depthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
	if (key == GLFW_KEY_P) isPerspective = (mods & GLFW_MOD_SHIFT) != 0;
	if (key == GLFW_KEY_1 || key == GLFW_KEY_KP_1) isOrbitScaling = !isOrbitScaling;
	if (key == GLFW_KEY_2 || key == GLFW_KEY_KP_2) isCenterAnimating = !isCenterAnimating;
	if (key == GLFW_KEY_3 || key == GLFW_KEY_KP_3) isMoonOrbitRotating = !isMoonOrbitRotating;

	if (key == GLFW_KEY_M) 
		isWireFace = (mods & GLFW_MOD_SHIFT) != 0;
	if (key == GLFW_KEY_R) ResetScene();

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
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
	DeleteMesh(sphereMesh);
	DeleteMesh(orbitMesh);
	glDeleteProgram(shaderProgram);
}
