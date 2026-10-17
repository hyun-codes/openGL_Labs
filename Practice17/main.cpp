#include "../PracticeStarterGL.h"
#include "ObjectAnimation.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <filesystem>
#include <limits>
#include <string>
#include <vector>

GLuint shaderProgram = 0;
Mesh coordinateMesh;
std::vector<Mesh> modelMeshes;
std::vector<std::string> modelNames;
ObjectState objects[2];
SwapAnimation swapAnimation;
GLFWwindow* mainWindow = nullptr;
int selectedObject = 2; // 0: 왼쪽 객체, 1: 오른쪽 객체, 2: 두 객체.
bool combinedAnimation = false;
float combinedScalePhase = 0.0f;
int framebufferWidth = 800;
int framebufferHeight = 800;

void InitCoordinateBuffer();
bool InitObjBuffer(const char* filePath, const glm::vec3& color, Mesh& mesh);
bool InitBuffer();
int FindModelIndex(const std::string& name);
void ResetScene();
void UpdateWindowTitle();
void ProcessTransformInput(float deltaTime);
void Update(float deltaTime);
void DrawScene();
void Cleanup();
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice17", nullptr, nullptr);
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
	std::cout << "1: left, 2: right, 3: both\n"
		<< "Hold x/y/r/a/b/d/e; Shift reverses the direction.\n"
		<< "t: swap through origin, u: curved swap, v: combined animation ON/OFF\n"
		<< "c: left cone / right torus, s: reset, q: quit" << std::endl;

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

void InitCoordinateBuffer()
{
	std::vector<Vertex> vertices = {
		{-1.5f, 0.0f, 0.0f, 1.0f, 0.2f, 0.2f}, {1.5f, 0.0f, 0.0f, 1.0f, 0.2f, 0.2f},
		{0.0f, -1.5f, 0.0f, 0.2f, 1.0f, 0.2f}, {0.0f, 1.5f, 0.0f, 0.2f, 1.0f, 0.2f},
		{0.0f, 0.0f, -1.5f, 0.2f, 0.4f, 1.0f}, {0.0f, 0.0f, 1.5f, 0.2f, 0.4f, 1.0f}
	};
	InitMeshBuffer(coordinateMesh, vertices, GL_LINES);
}

bool InitObjBuffer(const char* filePath, const glm::vec3& color, Mesh& mesh)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "OBJ 파일을 열 수 없습니다: " << filePath << std::endl;
		return false;
	}

	std::vector<glm::vec3> positions;
	std::vector<int> triangleIndices;
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream stream(line);
		std::string type;
		stream >> type;
		if (type == "v")
		{
			glm::vec3 position;
			if (stream >> position.x >> position.y >> position.z) positions.push_back(position);
		}
		else if (type == "f")
		{
			std::vector<int> face;
			std::string token;
			while (stream >> token)
			{
				if (token[0] == '#') break;
				// OBJ의 v/vt/vn 중 위치 인덱스만 사용한다.
				int index = 0;
				try { index = std::stoi(token.substr(0, token.find('/'))); }
				catch (...) { return false; }
				// OBJ는 1부터 시작하는 양수 인덱스와 끝에서 세는 음수 인덱스를 사용한다.
				index = index > 0 ? index - 1 : static_cast<int>(positions.size()) + index;
				if (index < 0 || static_cast<size_t>(index) >= positions.size()) return false;
				face.push_back(index);
			}
			if (face.size() < 3) return false;
			// 삼각형뿐 아니라 사각형 이상의 면도 삼각형 팬으로 나눈다.
			for (size_t i = 1; i + 1 < face.size(); ++i)
			{
				triangleIndices.insert(triangleIndices.end(), {face[0], face[i], face[i + 1]});
			}
		}
	}
	if (positions.empty() || triangleIndices.empty()) return false;
	glm::vec3 minimum(std::numeric_limits<float>::max());
	glm::vec3 maximum(std::numeric_limits<float>::lowest());
	for (const glm::vec3& p : positions)
	{
		minimum = glm::min(minimum, p);
		maximum = glm::max(maximum, p);
	}
	glm::vec3 center = (minimum + maximum) * 0.5f;
	float radius = 0.0f;
	for (const glm::vec3& p : positions) radius = std::max(radius, glm::length(p - center));
	if (radius < 0.00001f) return false;

	std::vector<Vertex> vertices;
	vertices.reserve(triangleIndices.size());
	for (int index : triangleIndices)
	{
		// 서로 다른 OBJ도 같은 중심과 반지름으로 정규화한다.
		glm::vec3 p = (positions[index] - center) / radius;
		// 좌표별 색상 차이를 주어 구의 자전도 눈으로 확인할 수 있게 한다.
		glm::vec3 vertexColor = color * (glm::vec3(0.4f) + 0.6f * (p + glm::vec3(1.0f)) * 0.5f);
		vertices.push_back({p.x, p.y, p.z, vertexColor.r, vertexColor.g, vertexColor.b});
	}
	InitMeshBuffer(mesh, vertices);
	return true;
}

bool InitBuffer()
{
	InitCoordinateBuffer();
	std::vector<std::filesystem::path> modelFiles;
	std::error_code error;
	std::filesystem::directory_iterator files("models", error);
	if (error)
	{
		std::cerr << "models 폴더를 열 수 없습니다: " << error.message() << std::endl;
		return false;
	}
	for (const auto& file : files)
		if (file.is_regular_file() && file.path().extension() == ".obj") modelFiles.push_back(file.path());
	std::sort(modelFiles.begin(), modelFiles.end());
	for (const auto& file : modelFiles)
	{
		Mesh mesh;
		if (!InitObjBuffer(file.string().c_str(), glm::vec3(1.0f), mesh))
		{
			std::cerr << "OBJ 로딩 실패: " << file << std::endl;
			return false;
		}
		modelMeshes.push_back(mesh);
		modelNames.push_back(file.stem().string());
		std::cout << "Loaded " << modelNames.back() << ": " << mesh.vertexCount << " vertices" << std::endl;
	}
	for (const char* name : {"sphere", "monkey", "cone", "torus"})
	{
		if (FindModelIndex(name) < 0)
		{
			std::cerr << "필요한 모델이 없습니다: models/" << name << ".obj" << std::endl;
			return false;
		}
	}
	return true;
}

int FindModelIndex(const std::string& name)
{
	auto found = std::find(modelNames.begin(), modelNames.end(), name);
	return found == modelNames.end() ? -1 : static_cast<int>(found - modelNames.begin());
}

void UpdateWindowTitle()
{
	const char* selected = selectedObject == 0 ? "Left" : selectedObject == 1 ? "Right" : "Both";
	std::string title = std::string("Practice17 | Selected: ") + selected + " | V: " + (combinedAnimation ? "ON" : "OFF");
	glfwSetWindowTitle(mainWindow, title.c_str());
}

void ResetScene()
{
	objects[0] = {};
	objects[1] = {};
	objects[0].position = glm::vec3(-0.9f, 0.0f, 0.0f);
	objects[1].position = glm::vec3(0.9f, 0.0f, 0.0f);
	objects[0].color = glm::vec3(0.65f, 0.85f, 1.0f);
	objects[1].color = glm::vec3(1.0f, 0.8f, 0.55f);
	objects[0].modelIndex = FindModelIndex("sphere");
	objects[1].modelIndex = FindModelIndex("monkey");
	selectedObject = 2;
	combinedAnimation = false;
	combinedScalePhase = 0.0f;
	swapAnimation = {};
	UpdateWindowTitle();
}

void ProcessTransformInput(float deltaTime)
{
	bool shift = glfwGetKey(mainWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(mainWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
	float direction = shift ? -1.0f : 1.0f;
	float angle = 60.0f * deltaTime * direction;
	float distance = 0.7f * deltaTime * direction;
	float scaleFactor = std::exp(0.65f * deltaTime * direction);
	for (int i = 0; i < 2; ++i)
	{
		if (selectedObject != 2 && selectedObject != i) continue;
		ObjectState& object = objects[i];
		if (glfwGetKey(mainWindow, GLFW_KEY_X) == GLFW_PRESS) RotateObject(object, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		if (glfwGetKey(mainWindow, GLFW_KEY_Y) == GLFW_PRESS) RotateObject(object, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		if (glfwGetKey(mainWindow, GLFW_KEY_R) == GLFW_PRESS) OrbitObject(object, angle);
		if (glfwGetKey(mainWindow, GLFW_KEY_A) == GLFW_PRESS) ScaleObject(object, scaleFactor, false);
		if (glfwGetKey(mainWindow, GLFW_KEY_B) == GLFW_PRESS) ScaleObject(object, scaleFactor, true);
		if (glfwGetKey(mainWindow, GLFW_KEY_D) == GLFW_PRESS) object.position.x -= distance;
		if (glfwGetKey(mainWindow, GLFW_KEY_E) == GLFW_PRESS) object.position.y += distance;
	}
}

void Update(float deltaTime)
{
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	UpdateSwap(swapAnimation, objects, deltaTime);
	if (combinedAnimation) UpdateCombinedAnimation(objects, combinedScalePhase, deltaTime);
	ProcessTransformInput(deltaTime);
}

void DrawScene()
{
	glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgram);

	glm::mat4 viewTransform = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f),
		glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	float aspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
	glm::mat4 projectionTransform = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	glm::mat4 baseTransform(1.0f);
	baseTransform = glm::rotate(baseTransform, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	baseTransform = glm::rotate(baseTransform, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	SetTransformUniforms(shaderProgram, baseTransform, viewTransform, projectionTransform);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 1.0f, 1.0f);
	DrawMesh(coordinateMesh);

	for (const ObjectState& object : objects)
	{
		glm::mat4 objectTransform = MakeObjectTransform(object, baseTransform);
		SetTransformUniforms(shaderProgram, objectTransform, viewTransform, projectionTransform);
		glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(object.color));
		DrawMesh(modelMeshes[object.modelIndex]);
	}
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;
	if (action != GLFW_PRESS) 
		return;
	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3)
	{
		selectedObject = key - GLFW_KEY_1;
		UpdateWindowTitle();
	}
	if (key == GLFW_KEY_T) 
		StartSwap(swapAnimation, objects, false);
	if (key == GLFW_KEY_U) 
		StartSwap(swapAnimation, objects, true);
	if (key == GLFW_KEY_V)
	{
		combinedAnimation = !combinedAnimation;
		UpdateWindowTitle();
	}
	if (key == GLFW_KEY_C)
	{
		objects[0].modelIndex = FindModelIndex("cone");
		objects[1].modelIndex = FindModelIndex("torus");
	}
	if (key == GLFW_KEY_S) 
		ResetScene();
	if (key == GLFW_KEY_Q)
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
	DeleteMesh(coordinateMesh);
	for (Mesh& mesh : modelMeshes) DeleteMesh(mesh);
	modelMeshes.clear();
	glDeleteProgram(shaderProgram);
}
