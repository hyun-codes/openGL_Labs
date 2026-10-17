#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
};

struct FacePair
{
	int firstFace;
	int secondFace;
	glm::vec3 edgeCenter;
};

enum class PyramidAnimationMode
{
	None,
	AllFaces,
	SequentialFaces
};

GLuint shaderProgram = 0;
GLuint coordinateVAO = 0;
GLuint coordinateVBO = 0;
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
GLuint pyramidVAO = 0;
GLuint pyramidVBO = 0;

std::string ReadShaderFile(const char* filePath);
GLuint CompileShader(GLenum shaderType, const char* filePath);
GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath);

void InitBuffer();
void InitCoordinateBuffer();
void InitCubeBuffer();
void InitPyramidBuffer();
void Update();
void DrawScene();
void DrawCoordinateScene();
void DrawCubeScene(const glm::mat4& objectTransform, GLint objectLocation);
void DrawCubeFace(int faceIndex, const glm::mat4& transform, GLint objectLocation);
void DrawPyramidScene(const glm::mat4& objectTransform, GLint objectLocation);
void DrawPyramidFace(int faceIndex, const glm::mat4& transform, GLint objectLocation);
void Cleanup();

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

bool drawCube = true;
bool depthTestEnabled = true;
bool yAxisRotate = false;
bool isTopFaceRotating = false;			//T키 토글.
bool isFrontFaceOpen = false;			//F키 토글.
bool isSideFaceRotating = false;		//S키 토글.
bool isBackFaceShrinking = false;		//B키 토글.
bool isTwoFaceRotating = false;			//1번 키.
bool isTwoFaceSwapping = false;			//2번 키.
bool isFourSideFacesOrbiting = false;	//4번 키.
bool isFourFacesOrbitingX = false;		//5번 키.
bool isAllPyramidFacesOpen = false;
bool isSequentialPyramidFacesOpen = false;
bool isOppositePyramidFacesSwapping = false;
bool isPyramidApexFacesOpen = false;

//y축에 대해서 자전.
float rotateAngleY = 0.0f;
float rotateVelocityY = 0.0f;
float topFaceRotateAngleZ = 0.0f;
float topFaceRotateVelocityZ = 0.0f;
float frontFaceRotateAngleX = 0.0f;
float sideFaceRotateAngleX = 0.0f;
float backFaceScale = 1.0f;
float twoFaceRotateAngleY = 0.0f;
float twoFaceSwapAngleY = 0.0f;
float fourSideFacesOrbitAngleY = 0.0f;
float fourFacesOrbitAngleX = 0.0f;
// 0: 앞면(0) + 뒷면(2), 1: 왼쪽 면(1) + 오른쪽 면(3).
float oppositePyramidFacesSwapAngleY[2] = {0.0f, 0.0f};
float oppositePyramidFacesSwapTargetAngleY[2] = {0.0f, 0.0f};
int selectedOppositePyramidPair = -1;
std::deque<int> pyramidSwapQueue;
float pyramidApexFacesOpenAngle = 0.0f;
float pyramidFaceOpenAngles[4] = {
	0.0f,
	0.0f,
	0.0f,
	0.0f
};

int currentSequentialFace = 0;
PyramidAnimationMode pyramidAnimationMode = PyramidAnimationMode::None;

//초기 세팅 이동 속도.
const float rotateSpeed = 60.0f;
const float scaleSpeed = 1.0f;

double lastFrameTime = 0.0f;

const FacePair facePairs[4] = {
	//앞면 + 왼쪽 면
	{ 0, 2, glm::vec3(-0.3f, 0.0f, -0.3f) },
	//앞면 + 오른쪽 면
	{ 0, 3, glm::vec3(0.3f, 0.0f, -0.3f) },
	//뒷면 + 왼쪽 면
	{ 1, 2, glm::vec3(-0.3f, 0.0f, 0.3f) },
	//뒷면 + 오른쪽 면
	{ 1, 3, glm::vec3(0.3f, 0.0f, 0.3f) }
};

const float swapDirections[4] = {
	 1.0f,
	-1.0f,
	-1.0f,
	 1.0f
};

int selectedFacePair = -1;			//4가지의 경우의 수.
int selectedSwapPair = -1;			//2번에서 선택한 두 면.

int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// GLFW 초기화
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	// OpenGL 3.3 Core Profile 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice16", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallBack);
	glfwSetFramebufferSizeCallback(window, framebufferCallBack);

	// GLEW 초기화
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 800);
	glEnable(GL_DEPTH_TEST);

	// 셰이더 프로그램 생성
	shaderProgram = CreateShaderProgram(
		"shaders/vertex.glsl",
		"shaders/fragment.glsl");

	if (shaderProgram == 0)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	InitBuffer();
	lastFrameTime = glfwGetTime();

	std::cout << "Vendor: "
		<< glGetString(GL_VENDOR) << std::endl;

	std::cout << "Renderer: "
		<< glGetString(GL_RENDERER) << std::endl;

	std::cout << "OpenGL Version: "
		<< glGetString(GL_VERSION) << std::endl;

	// 메인 루프
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

std::string ReadShaderFile(const char* filePath)
{
	std::ifstream shaderFile(filePath);
	if (!shaderFile.is_open())
	{
		std::cerr << "셰이더 파일을 열 수 없습니다: " << filePath << std::endl;
		return "";
	}

	std::stringstream shaderStream;
	shaderStream << shaderFile.rdbuf();
	return shaderStream.str();
}

GLuint CompileShader(GLenum shaderType, const char* filePath)
{
	std::string shaderSource = ReadShaderFile(filePath);
	if (shaderSource.empty())
		return 0;

	const char* source = shaderSource.c_str();
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint success = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		char infoLog[1024] = {};
		glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
		std::cerr << filePath << " 컴파일 실패!\n" << infoLog << std::endl;
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath)
{
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexPath);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentPath);
	if (vertexShader == 0 || fragmentShader == 0)
	{
		if (vertexShader != 0)
			glDeleteShader(vertexShader);
		if (fragmentShader != 0)
			glDeleteShader(fragmentShader);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint success = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		char infoLog[1024] = {};
		glGetProgramInfoLog(program, 1024, nullptr, infoLog);
		std::cerr << "셰이더 프로그램 링크 실패!\n" << infoLog << std::endl;
		glDeleteProgram(program);
		program = 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

void InitBuffer()
{
	InitCoordinateBuffer();
	InitCubeBuffer();
	InitPyramidBuffer();
}

void InitCoordinateBuffer()
{
	Vertex coordinatesVertex[] = {
		// x축: 빨강
		{-0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		{ 0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		// y축: 초록
		{0.0f, -0.8f, 0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f,  0.8f, 0.0f, 0.0f, 1.0f, 0.0f},
		// z축: 파랑
		{0.0f, 0.0f, -0.8f, 0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f,  0.8f, 0.0f, 0.0f, 1.0f}
	};

	glGenVertexArrays(1, &coordinateVAO);
	glBindVertexArray(coordinateVAO);

	glGenBuffers(1, &coordinateVBO);
	glBindBuffer(GL_ARRAY_BUFFER, coordinateVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coordinatesVertex), coordinatesVertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void InitCubeBuffer()
{
	Vertex cubeVertices[] = {
		// 앞면
		{ -0.3f,  0.3f, -0.3f,  1.0f, 0.0f, 0.0f },
		{  0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },

		{ -0.3f,  0.3f, -0.3f,  1.0f, 0.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },
		{ -0.3f, -0.3f, -0.3f,  1.0f, 1.0f, 0.0f },

		// 뒷면
		{ -0.3f,  0.3f,  0.3f,  0.0f, 1.0f, 1.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.5f, 0.0f },

		{ -0.3f,  0.3f,  0.3f,  0.0f, 1.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.5f, 0.0f },
		{  0.3f,  0.3f,  0.3f,  1.0f, 0.0f, 1.0f },

		// 왼쪽 면
		{ -0.3f,  0.3f, -0.3f,  1.0f, 0.0f, 0.0f },
		{ -0.3f, -0.3f, -0.3f,  1.0f, 1.0f, 0.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 1.0f },

		{ -0.3f,  0.3f, -0.3f,  1.0f, 0.0f, 0.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 1.0f },
		{ -0.3f,  0.3f,  0.3f,  0.0f, 1.0f, 1.0f },

		// 오른쪽 면
		{  0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f,  0.3f,  0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.5f, 0.0f },

		{  0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.5f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },

		// 윗면
		{ -0.3f,  0.3f, -0.3f,  1.0f, 0.0f, 0.0f },
		{ -0.3f,  0.3f,  0.3f,  0.0f, 1.0f, 1.0f },
		{  0.3f,  0.3f,  0.3f,  1.0f, 0.0f, 1.0f },

		{ -0.3f,  0.3f, -0.3f,  1.0f, 0.0f, 0.0f },
		{  0.3f,  0.3f,  0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 0.0f },

		// 아랫면
		{ -0.3f, -0.3f, -0.3f,  1.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.5f, 0.0f },

		{ -0.3f, -0.3f, -0.3f,  1.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.5f, 0.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 1.0f }
	};

	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void InitPyramidBuffer()
{
	Vertex pyramidVertices[] = {
		// 앞쪽 삼각형
		// 꼭짓점, 오른쪽 아래, 왼쪽 아래
		{  0.0f,  0.4f,  0.0f,  1.0f, 0.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },
		{ -0.3f, -0.3f, -0.3f,  0.0f, 1.0f, 0.0f },

		// 왼쪽 삼각형
		// 꼭짓점, 왼쪽 앞, 왼쪽 뒤
		{  0.0f,  0.4f,  0.0f,  1.0f, 0.0f, 0.0f },
		{ -0.3f, -0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 1.0f },

		// 뒤쪽 삼각형
		// 꼭짓점, 왼쪽 뒤, 오른쪽 뒤
		{  0.0f,  0.4f,  0.0f,  1.0f, 0.0f, 0.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 0.0f },

		// 오른쪽 삼각형
		// 꼭짓점, 오른쪽 뒤, 오른쪽 앞
		{  0.0f,  0.4f,  0.0f,  1.0f, 0.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },

		// 바닥면 첫 번째 삼각형
		{ -0.3f, -0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 0.0f },

		// 바닥면 두 번째 삼각형
		{ -0.3f, -0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 0.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 1.0f }
	};

	glGenVertexArrays(1, &pyramidVAO);
	glBindVertexArray(pyramidVAO);

	glGenBuffers(1, &pyramidVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Update()
{
	double currentTime = glfwGetTime();

	float deltaTime = static_cast<float>(currentTime - lastFrameTime);

	lastFrameTime = currentTime;
	//y축 기준 회전.
	rotateAngleY += rotateVelocityY * deltaTime;
	//윗면 가운데 z축 회전.
	topFaceRotateAngleZ += topFaceRotateVelocityZ * deltaTime;
	//앞면 위축 회전.
	if (isFrontFaceOpen){
		frontFaceRotateAngleX += rotateSpeed * deltaTime;

		if (frontFaceRotateAngleX > 90.0f)
			frontFaceRotateAngleX = 90.0f;
	}
	else
	{
		frontFaceRotateAngleX -= rotateSpeed * deltaTime;

		if (frontFaceRotateAngleX < 0.0f)
			frontFaceRotateAngleX = 0.0f;
	}
	//옆면 중점 회전.
	if (isSideFaceRotating)
		sideFaceRotateAngleX += rotateSpeed * deltaTime;

	//뒷면 축소 확대.
	if (isBackFaceShrinking) {
		backFaceScale -= scaleSpeed * deltaTime;
		if (backFaceScale < 0.0f)
			backFaceScale = 0.0f;
	}
	else {
		backFaceScale += scaleSpeed * deltaTime;
		if (backFaceScale > 1.0f)
			backFaceScale = 1.0f;
	}
	//선택된 두 면을 공통 모서리 기준으로 360도 회전. - 1번 키.
	if (isTwoFaceRotating) {
		twoFaceRotateAngleY += rotateSpeed * deltaTime;

		if (twoFaceRotateAngleY >= 360.0f) {
			twoFaceRotateAngleY = 0.0f;
			isTwoFaceRotating = false;
			selectedFacePair = -1;
		}
	}
	//선택된 두면이 중점(0,0,0)을 기준으로 90도 회전. - 2번 키.
	if (isTwoFaceSwapping) {
		twoFaceSwapAngleY += rotateSpeed * deltaTime;

		if (twoFaceSwapAngleY >= 90.0f) {
			twoFaceSwapAngleY = 90.0f;
			isTwoFaceSwapping = false;
		}
	}
	//4개의 옆면이 y축을 기준으로 공전. - 4번 키.
	if (isFourSideFacesOrbiting)
	{
		fourSideFacesOrbitAngleY +=
			rotateSpeed * deltaTime;

		if (fourSideFacesOrbitAngleY >= 360.0f)
			fourSideFacesOrbitAngleY -= 360.0f;
	}
	//윗면, 아랫면, 앞, 뒤면이 x축을 기준으로 공전 - 5번 키.
	if (isFourFacesOrbitingX)
	{
		fourFacesOrbitAngleX +=
			rotateSpeed * deltaTime;
		if (fourFacesOrbitAngleX >= 360.0f)
			fourFacesOrbitAngleX -= 360.0f;
	}
	//피라미드.
	//6번 애니메이션. - 사각뿔의 옆면들이 모서리들을 기준으로 동시에 열린다/닫힌다.
	if (pyramidAnimationMode == PyramidAnimationMode::AllFaces)
	{
		float targetAngle = isAllPyramidFacesOpen ? 180.0f : 0.0f;
		bool animationFinished = true;

		for (int i = 0; i < 4; ++i)
		{
			if (pyramidFaceOpenAngles[i] < targetAngle)
			{
				pyramidFaceOpenAngles[i] += rotateSpeed * deltaTime;

				if (pyramidFaceOpenAngles[i] > targetAngle)
					pyramidFaceOpenAngles[i] = targetAngle;
			}
			else if (pyramidFaceOpenAngles[i] > targetAngle)
			{
				pyramidFaceOpenAngles[i] -= rotateSpeed * deltaTime;

				if (pyramidFaceOpenAngles[i] < targetAngle)
					pyramidFaceOpenAngles[i] = targetAngle;
			}

			if (pyramidFaceOpenAngles[i] != targetAngle)
				animationFinished = false;
		}

		if (animationFinished)
			pyramidAnimationMode = PyramidAnimationMode::None;
	}

	//7번 애니메이션. - 사각뿔의 옆면들이 한 면씩 90도까지 열린다/닫힌다.
	if (pyramidAnimationMode == PyramidAnimationMode::SequentialFaces)
	{
		float targetAngle = isSequentialPyramidFacesOpen ? 90.0f : 0.0f;
		float& currentAngle = pyramidFaceOpenAngles[currentSequentialFace];

		if (currentAngle < targetAngle)
		{
			currentAngle += rotateSpeed * deltaTime;

			if (currentAngle > targetAngle)
				currentAngle = targetAngle;
		}
		else if (currentAngle > targetAngle)
		{
			currentAngle -= rotateSpeed * deltaTime;

			if (currentAngle < targetAngle)
				currentAngle = targetAngle;
		}

		if (currentAngle == targetAngle)
		{
			if (isSequentialPyramidFacesOpen)
				++currentSequentialFace;
			else
				--currentSequentialFace;

			if (currentSequentialFace < 0 || currentSequentialFace >= 4)
			{
				currentSequentialFace = isSequentialPyramidFacesOpen ? 3 : 0;
				pyramidAnimationMode = PyramidAnimationMode::None;
			}
		}
	}

	//8번 애니메이션. - 마주 보는 옆면들이 꼭짓점의 y축을 기준으로 자리를 바꾼다.
	if (!isOppositePyramidFacesSwapping && !pyramidSwapQueue.empty())
	{
		selectedOppositePyramidPair = pyramidSwapQueue.front();
		pyramidSwapQueue.pop_front();
		oppositePyramidFacesSwapTargetAngleY[selectedOppositePyramidPair] =
			oppositePyramidFacesSwapAngleY[selectedOppositePyramidPair] + 180.0f;
		isOppositePyramidFacesSwapping = true;
	}
	if (isOppositePyramidFacesSwapping)
	{
		float& angle = oppositePyramidFacesSwapAngleY[selectedOppositePyramidPair];
		float& targetAngle = oppositePyramidFacesSwapTargetAngleY[selectedOppositePyramidPair];
		angle += rotateSpeed * deltaTime;

		if (angle >= targetAngle)
		{
			angle = targetAngle;
			if (angle >= 360.0f) angle -= 360.0f;
			targetAngle = angle;
			isOppositePyramidFacesSwapping = false;
		}
	}

	//9번 애니메이션. - 사각뿔의 옆면들이 꼭짓점을 기준으로 바깥쪽으로 열린다/닫힌다.
	float pyramidApexFacesTargetAngle = isPyramidApexFacesOpen ? 90.0f : 0.0f;

	if (pyramidApexFacesOpenAngle < pyramidApexFacesTargetAngle)
	{
		pyramidApexFacesOpenAngle += rotateSpeed * deltaTime;

		if (pyramidApexFacesOpenAngle > pyramidApexFacesTargetAngle)
			pyramidApexFacesOpenAngle = pyramidApexFacesTargetAngle;
	}
	else if (pyramidApexFacesOpenAngle > pyramidApexFacesTargetAngle)
	{
		pyramidApexFacesOpenAngle -= rotateSpeed * deltaTime;

		if (pyramidApexFacesOpenAngle < pyramidApexFacesTargetAngle)
			pyramidApexFacesOpenAngle = pyramidApexFacesTargetAngle;
	}
}

void DrawScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	// 좌표축과 객체에 적용할 기본 회전행렬
	glm::mat4 baseTransform(1.0f);
	/*baseTransform = glm::rotate(
		baseTransform,
		glm::radians(-30.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);
	baseTransform = glm::rotate(
		baseTransform,
		glm::radians(-30.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);*/

	GLint baseLocation = glGetUniformLocation(shaderProgram, "modelTransform");
	glUniformMatrix4fv(
		baseLocation,
		1,
		GL_FALSE,
		glm::value_ptr(baseTransform)
	);

	DrawCoordinateScene();

	glm::mat4 objectTransform = baseTransform;

	objectTransform = glm::rotate(
		objectTransform,
		glm::radians(rotateAngleY),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	GLint objectLocation = glGetUniformLocation(shaderProgram, "modelTransform");
	glUniformMatrix4fv(
		objectLocation,
		1,
		GL_FALSE,
		glm::value_ptr(objectTransform)
	);

	if (drawCube)
		DrawCubeScene(objectTransform, objectLocation);
	else
		DrawPyramidScene(objectTransform, objectLocation);

	glUseProgram(0);
}

void DrawCoordinateScene()
{
	glBindVertexArray(coordinateVAO);
	glDrawArrays(GL_LINES, 0, 6);
	glBindVertexArray(0);
}

void DrawCubeFace(int faceIndex, const glm::mat4& transform, GLint objectLocation)
{
	glUniformMatrix4fv(
		objectLocation,
		1,
		GL_FALSE,
		glm::value_ptr(transform)
	);

	glDrawArrays(
		GL_TRIANGLES,
		faceIndex * 6,
		6
	);
}

void DrawPyramidFace(int faceIndex, const glm::mat4& transform, GLint objectLocation)
{
	glUniformMatrix4fv(
		objectLocation,
		1,
		GL_FALSE,
		glm::value_ptr(transform)
	);

	if (faceIndex < 4)
	{
		//옆면은 한 면당 정점 3개
		glDrawArrays(
			GL_TRIANGLES,
			faceIndex * 3,
			3
		);
	}
	else
	{
		//밑면은 마지막 정점 6개
		glDrawArrays(
			GL_TRIANGLES,
			12,
			6
		);
	}
}

void DrawCubeScene(const glm::mat4& objectTransform, GLint objectLocation)
{
	glBindVertexArray(cubeVAO);

	//각 면의 개별 애니메이션 행렬.
	glm::mat4 faceTransforms[6] = {
		glm::mat4(1.0f),
		glm::mat4(1.0f),
		glm::mat4(1.0f),
		glm::mat4(1.0f),
		glm::mat4(1.0f),
		glm::mat4(1.0f)
	};

	//앞면을 그리기 위한 행렬.
	glm::mat4 frontFaceTransform(1.0f);
	frontFaceTransform = glm::translate(
		frontFaceTransform,
		glm::vec3(0.0f, 0.3f, -0.3f)
	);

	frontFaceTransform = glm::rotate(
		frontFaceTransform,
		glm::radians(frontFaceRotateAngleX),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	frontFaceTransform = glm::translate(
		frontFaceTransform,
		glm::vec3(0.0f, -0.3f, 0.3f)
	);
	faceTransforms[0] = frontFaceTransform;

	//뒷면을 위한 변환 행렬.
	glm::vec3 backFaceCenter(0.0f, 0.0f, 0.3f);
	glm::mat4 backFaceTransform(1.0f);
	backFaceTransform = glm::translate(
		backFaceTransform,
		backFaceCenter
	);
	backFaceTransform = glm::scale(
		backFaceTransform,
		glm::vec3(
			backFaceScale,
			backFaceScale,
			1.0f
		)
	);
	backFaceTransform = glm::translate(
		backFaceTransform,
		-backFaceCenter
	);
	faceTransforms[1] = backFaceTransform;

	//옆면을 위한 회전행렬.
	glm::mat4 sideFaceTransform(1.0f);
	sideFaceTransform = glm::rotate(
		sideFaceTransform,
		glm::radians(sideFaceRotateAngleX),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);
	faceTransforms[2] = sideFaceTransform;
	faceTransforms[3] = sideFaceTransform;

	//윗면을 위한 회전행렬.
	glm::mat4 topFaceTransform(1.0f);
	topFaceTransform = glm::translate(
		topFaceTransform,
		glm::vec3(0.0f, 0.3f, 0.0f)
	);

	topFaceTransform = glm::rotate(
		topFaceTransform,
		glm::radians(topFaceRotateAngleZ),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	topFaceTransform = glm::translate(
		topFaceTransform,
		glm::vec3(0.0f, -0.3f, 0.0f)
	);
	faceTransforms[4] = topFaceTransform;

	//1번에서 선택된 두 면의 공통 모서리 회전행렬.
	glm::mat4 twoFaceTransform(1.0f);
	if (isTwoFaceRotating && selectedFacePair >= 0) {
		const FacePair& pair = facePairs[selectedFacePair];

		twoFaceTransform = glm::translate(
			twoFaceTransform,
			pair.edgeCenter
		);
		twoFaceTransform = glm::rotate(
			twoFaceTransform,
			glm::radians(twoFaceRotateAngleY),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		twoFaceTransform = glm::translate(
			twoFaceTransform,
			-pair.edgeCenter
		);
	}
	//2번에서 선택된 각각의 두면의 회전행렬.
	glm::mat4 firstFaceSwapTransform(1.0f);
	glm::mat4 secondFaceSwapTransform(1.0f);
	if (selectedSwapPair >= 0) {
		float direction = swapDirections[selectedSwapPair];

		firstFaceSwapTransform = glm::rotate(
			firstFaceSwapTransform,
			glm::radians(twoFaceSwapAngleY * direction),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		secondFaceSwapTransform = glm::rotate(
			secondFaceSwapTransform,
			glm::radians(-twoFaceSwapAngleY * direction),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}

	//4번에서 육면체의 4개의 옆면이 공전하는 회전행렬.
	glm::mat4 fourSideFacesOrbitTransform(1.0f);
	fourSideFacesOrbitTransform = glm::rotate(
		fourSideFacesOrbitTransform,
		glm::radians(fourSideFacesOrbitAngleY),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	//5번에서 x축을 기준으로 그 주변 면 공전하는 회전행렬.
	glm::mat4 fourFacesOrbitTransformX(1.0f);
	fourFacesOrbitTransformX = glm::rotate(
		fourFacesOrbitTransformX,
		glm::radians(fourFacesOrbitAngleX),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	//각 면의 개별 변환과 1번의 두 면 변환을 결합하여 한 번씩 그린다.
	for (int faceIndex = 0; faceIndex < 6; ++faceIndex) {
		glm::mat4 faceTransform = objectTransform;

		if (isTwoFaceRotating && selectedFacePair >= 0) {
			const FacePair& pair = facePairs[selectedFacePair];
			if (faceIndex == pair.firstFace || faceIndex == pair.secondFace)
				faceTransform = faceTransform * twoFaceTransform;
		}

		if (selectedSwapPair >= 0) {
			const FacePair& pair = facePairs[selectedSwapPair];
			if (faceIndex == pair.firstFace) 
				faceTransform = faceTransform * firstFaceSwapTransform;
			if (faceIndex == pair.secondFace)
				faceTransform = faceTransform * secondFaceSwapTransform;
		}

		if (isFourSideFacesOrbiting)
		{
			if(faceIndex <= 3)
				faceTransform = faceTransform * fourSideFacesOrbitTransform;
		}
		
		if (isFourFacesOrbitingX) {
			if (faceIndex == 0 || faceIndex == 1 ||
				faceIndex == 4 || faceIndex == 5)
				faceTransform = faceTransform * fourFacesOrbitTransformX;
		}
			
		faceTransform = faceTransform * faceTransforms[faceIndex];
		DrawCubeFace(faceIndex, faceTransform, objectLocation);
	}

	glBindVertexArray(0);
}

void DrawPyramidScene(const glm::mat4& objectTransform, GLint objectLocation)
{
	glBindVertexArray(pyramidVAO);

	//8번에서 선택한 쌍만 사각뿔의 꼭짓점을 기준으로 회전한다.
	glm::vec3 pyramidTop(0.0f, 0.4f, 0.0f);

	for (int faceIndex = 0; faceIndex < 4; ++faceIndex)
	{
		int pairIndex = faceIndex % 2;
		glm::mat4 oppositePyramidFacesSwapTransform(1.0f);
		oppositePyramidFacesSwapTransform = glm::translate(oppositePyramidFacesSwapTransform, pyramidTop);
		oppositePyramidFacesSwapTransform = glm::rotate(
			oppositePyramidFacesSwapTransform,
			glm::radians(oppositePyramidFacesSwapAngleY[pairIndex]),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		oppositePyramidFacesSwapTransform = glm::translate(oppositePyramidFacesSwapTransform, -pyramidTop);
		glm::mat4 faceTransform = objectTransform;
		glm::mat4 pyramidFaceTransform(1.0f);
		glm::vec3 pyramidApexRotationAxis(1.0f, 0.0f, 0.0f);
		float pyramidApexRotationDirection = 1.0f;

		switch (faceIndex)
		{
		//앞면.
		case 0:
			pyramidApexRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
			pyramidApexRotationDirection = 1.0f;

			pyramidFaceTransform = glm::translate(
				pyramidFaceTransform,
				glm::vec3(0.0f, -0.3f, -0.3f)
			);
			pyramidFaceTransform = glm::rotate(
				pyramidFaceTransform,
				glm::radians(-pyramidFaceOpenAngles[faceIndex]),
				glm::vec3(1.0f, 0.0f, 0.0f)
			);
			pyramidFaceTransform = glm::translate(
				pyramidFaceTransform,
				glm::vec3(0.0f, 0.3f, 0.3f)
			);
			break;

		case 1:
			pyramidApexRotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
			pyramidApexRotationDirection = -1.0f;

			pyramidFaceTransform = glm::translate(
				pyramidFaceTransform,
				glm::vec3(-0.3f, -0.3f, 0.0f)
			);
			pyramidFaceTransform = glm::rotate(
				pyramidFaceTransform,
				glm::radians(pyramidFaceOpenAngles[faceIndex]),
				glm::vec3(0.0f, 0.0f, 1.0f)
			);
			pyramidFaceTransform = glm::translate(
				pyramidFaceTransform,
				glm::vec3(0.3f, 0.3f, 0.0f)
			);
			break;

		case 2:
			pyramidApexRotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
			pyramidApexRotationDirection = -1.0f;

			pyramidFaceTransform = glm::translate(
				pyramidFaceTransform,
				glm::vec3(0.0f, -0.3f, 0.3f)
			);
			pyramidFaceTransform = glm::rotate(
				pyramidFaceTransform,
				glm::radians(pyramidFaceOpenAngles[faceIndex]),
				glm::vec3(1.0f, 0.0f, 0.0f)
			);
			pyramidFaceTransform = glm::translate(
				pyramidFaceTransform,
				glm::vec3(0.0f, 0.3f, -0.3f)
			);
			break;

		case 3:
			pyramidApexRotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
			pyramidApexRotationDirection = 1.0f;

			pyramidFaceTransform = glm::translate(
				pyramidFaceTransform,
				glm::vec3(0.3f, -0.3f, 0.0f)
			);
			pyramidFaceTransform = glm::rotate(
				pyramidFaceTransform,
				glm::radians(-pyramidFaceOpenAngles[faceIndex]),
				glm::vec3(0.0f, 0.0f, 1.0f)
			);
			pyramidFaceTransform = glm::translate(
				pyramidFaceTransform,
				glm::vec3(-0.3f, 0.3f, 0.0f)
			);
			break;
		}

		glm::mat4 pyramidApexFaceOpenTransform(1.0f);
		pyramidApexFaceOpenTransform = glm::translate(
			pyramidApexFaceOpenTransform,
			pyramidTop
		);
		pyramidApexFaceOpenTransform = glm::rotate(
			pyramidApexFaceOpenTransform,
			glm::radians(
				pyramidApexFacesOpenAngle * pyramidApexRotationDirection
			),
			pyramidApexRotationAxis
		);
		pyramidApexFaceOpenTransform = glm::translate(
			pyramidApexFaceOpenTransform,
			-pyramidTop
		);

		faceTransform = faceTransform
			* oppositePyramidFacesSwapTransform
			* pyramidApexFaceOpenTransform
			* pyramidFaceTransform;

		DrawPyramidFace(faceIndex, faceTransform, objectLocation);
	}

	glUniformMatrix4fv(
		objectLocation,
		1,
		GL_FALSE,
		glm::value_ptr(objectTransform)
	);

	glDrawArrays(
		GL_TRIANGLES,
		12,
		6
	);

	glBindVertexArray(0);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;

	if (action != GLFW_PRESS)
		return;
	//은면 제거 설정/해제.
	if (key == GLFW_KEY_H) {
		depthTestEnabled = !depthTestEnabled;

		if (depthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
	//육면체/사각뿔 토글
	if (key == GLFW_KEY_P) {
		drawCube = !drawCube;
	}
	//y축 회전.
	if (key == GLFW_KEY_Y) {
		yAxisRotate = !yAxisRotate;
		if (yAxisRotate)
			rotateVelocityY = rotateSpeed;
		else
			rotateVelocityY = 0.0f;
	}

	if (key == GLFW_KEY_R) {
		rotateAngleY = 0.0f;
		topFaceRotateAngleZ = 0.0f;
		frontFaceRotateAngleX = 0.0f;
		sideFaceRotateAngleX = 0.0f;

		rotateVelocityY = 0.0f;
		topFaceRotateVelocityZ = 0.0f;
		backFaceScale = 1.0f;

		yAxisRotate = false;
		isTopFaceRotating = false;
		isFrontFaceOpen = false;
		isSideFaceRotating = false;
		isBackFaceShrinking = false;
		isTwoFaceRotating = false;
		isTwoFaceSwapping = false;
		isFourSideFacesOrbiting = false;
		isAllPyramidFacesOpen = false;
		isSequentialPyramidFacesOpen = false;
		isOppositePyramidFacesSwapping = false;
		isPyramidApexFacesOpen = false;
		pyramidAnimationMode = PyramidAnimationMode::None;
		currentSequentialFace = 0;

		twoFaceRotateAngleY = 0.0f;
		twoFaceSwapAngleY = 0.0f;
		fourSideFacesOrbitAngleY = 0.0f;
		for (int pairIndex = 0; pairIndex < 2; ++pairIndex)
		{
			oppositePyramidFacesSwapAngleY[pairIndex] = 0.0f;
			oppositePyramidFacesSwapTargetAngleY[pairIndex] = 0.0f;
		}
		selectedOppositePyramidPair = -1;
		pyramidSwapQueue.clear();
		pyramidApexFacesOpenAngle = 0.0f;
		for (int i = 0; i < 4; ++i)
			pyramidFaceOpenAngles[i] = 0.0f;

		selectedFacePair = -1;
		selectedSwapPair = -1;

		drawCube = true;
		depthTestEnabled = true;
		glEnable(GL_DEPTH_TEST);
	}

	if (key == GLFW_KEY_T) {
		isTopFaceRotating = !isTopFaceRotating;
		if (isTopFaceRotating) {
			topFaceRotateVelocityZ = rotateSpeed;
		}
		else {
			topFaceRotateVelocityZ = 0.0f;
		}
	}
	if (key == GLFW_KEY_F)
		isFrontFaceOpen = !isFrontFaceOpen;

	if (key == GLFW_KEY_S)
		isSideFaceRotating = !isSideFaceRotating;

	if (key == GLFW_KEY_B)
		isBackFaceShrinking = !isBackFaceShrinking;

	if (key == GLFW_KEY_1 && !isTwoFaceRotating) {
		selectedFacePair = std::rand() % 4;
		twoFaceRotateAngleY = 0.0f;
		isTwoFaceRotating = true;
	}

	if (key == GLFW_KEY_2 && !isTwoFaceSwapping) {
		selectedSwapPair = std::rand() % 4;
		twoFaceSwapAngleY = 0.0f;
		isTwoFaceSwapping = true;
	}

	if (key == GLFW_KEY_4 && !isFourSideFacesOrbiting) {
		fourSideFacesOrbitAngleY = 0.0f;
		isFourSideFacesOrbiting = true;
	}

	if (key == GLFW_KEY_5 && !isFourFacesOrbitingX) {
		fourFacesOrbitAngleX = 0.0f;
		isFourFacesOrbitingX = true;
	}

	if (key == GLFW_KEY_6 && !drawCube) {
		isAllPyramidFacesOpen = !isAllPyramidFacesOpen;
		pyramidAnimationMode = PyramidAnimationMode::AllFaces;
	}
	
	if (key == GLFW_KEY_7 && !drawCube) {
		isSequentialPyramidFacesOpen = !isSequentialPyramidFacesOpen;
		currentSequentialFace = isSequentialPyramidFacesOpen ? 0 : 3;
		pyramidAnimationMode = PyramidAnimationMode::SequentialFaces;
	}
	if (key == GLFW_KEY_8 && !drawCube) {
		// 연속 입력도 빠뜨리지 않고 한 쌍씩 순서대로 수행한다.
		pyramidSwapQueue.push_back(std::rand() % 2);
	}
	
	if (key == GLFW_KEY_9 && !drawCube) {
		isPyramidApexFacesOpen = !isPyramidApexFacesOpen;
	}
	if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebufferCallBack(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

void Cleanup()
{
	if (coordinateVBO != 0)
		glDeleteBuffers(1, &coordinateVBO);
	if (coordinateVAO != 0)
		glDeleteVertexArrays(1, &coordinateVAO);
	if (cubeVBO != 0)
		glDeleteBuffers(1, &cubeVBO);
	if (cubeVAO != 0)
		glDeleteVertexArrays(1, &cubeVAO);
	if (pyramidVBO != 0)
		glDeleteBuffers(1, &pyramidVBO);
	if (pyramidVAO != 0)
		glDeleteVertexArrays(1, &pyramidVAO);
	if (shaderProgram != 0)
		glDeleteProgram(shaderProgram);
}
