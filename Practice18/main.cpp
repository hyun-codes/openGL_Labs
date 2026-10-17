#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

struct Camera
{
	glm::vec3 position{0.0f, 2.0f, 6.0f};	//위치는 z축 4 위치.
	glm::vec3 target{ 0.0f, 0.0f, 0.0f };	//타겟은 원점.
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	float fovDegrees = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
};

GLuint shaderProgram = 0;

Mesh sphereMesh;
Mesh orbitMesh;
Camera camera;

int framebufferWidth = 800;
int framebufferHeight = 800;

const float PI = 3.14159265359f;
float orbitRadius = 2.0f;

double lastFrameTime = 0.0f;

bool depthTestEnabled = true;
bool isWireFace = false;

bool LoadObjVertices(const char* filePath, std::vector<Vertex>& vertices);
bool InitBuffer();
void InitOrbitBuffer();
void Update();
void DrawScene();
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
	if (!InitBuffer())
	{
		Cleanup();
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	lastFrameTime = glfwGetTime();
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
		float x = orbitRadius * std::cos(theta);
		float z = orbitRadius * std::sin(theta);
		orbitPosition.push_back({ x, 0, z, 1.0, 1.0f, 1.0f });
	}	
	InitMeshBuffer(orbitMesh, orbitPosition, GL_LINE_LOOP);
}

void Update()
{
	double currentTime = glfwGetTime();
	float deltaTime = static_cast<float>(currentTime - lastFrameTime);
	lastFrameTime = currentTime;

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
	glm::mat4 projection = glm::perspective(glm::radians(camera.fovDegrees), aspect, camera.nearPlane, camera.farPlane);

	// 카메라와 투영은 모든 도형이 공유한다. 중심 구의 모델 행렬부터 전달한다.
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.45f));
	SetTransformUniforms(shaderProgram, model, view, projection);
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.0f, 0.0f, 1.0f);
	DrawMesh(sphereMesh);

	//궤도 x,y축을 기준으로 0도, 45도, -45도 넘겨주기.
	for (float orbitAngle : {0.0f, 45.0f, -45.0f})
	{
		glm::mat4 orbitTransform = glm::rotate(
			glm::mat4(1.0f),
			glm::radians(orbitAngle),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		DrawOrbitScene(orbitTransform, glm::vec3(0.6f));
	}
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
	(void)mods;

	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_P) {
		depthTestEnabled = !depthTestEnabled;

		if (depthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	if (key == GLFW_KEY_M) 
		isWireFace = (mods & GLFW_MOD_SHIFT) != 0;

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
	glDeleteProgram(shaderProgram);
}
