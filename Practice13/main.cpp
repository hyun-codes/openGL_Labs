#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <ctime>

struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
};
enum CubeFace
{
	Front = 0,
	Back,
	Left,
	Right,
	Top,
	Bottom
};

bool drawCubeFace[6] = {
	false, false, false,
	false, false, false
};

bool drawPyramidFace[4] = {
	//7, 8, 9, 0
	false, false, false, false
};
bool drawRandomCube = false;
bool drawPyramidBottom = false;

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
void DrawCubeScene();
void DrawPyramidScene();
void Cleanup();

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);

int main()
{
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
	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice13", nullptr, nullptr);
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
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

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
		//x축 - Red.
		{-0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		{0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		//y축 - Green
		{0.0f, -0.8f, 0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.8f, 0.0f, 0.0f, 1.0f, 0.0f},
		//z축 - Blue
		{0.0f, 0.0f, -0.8f, 0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 0.8f, 0.0f, 0.0f, 1.0f}
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

	//bind된거 해제.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void InitCubeBuffer()
{
	Vertex cubeVertices[] = {
		// 1번 앞면: 초록, z = -0.3
		{ -0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 1.0f, 0.0f },

		{ -0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  0.0f, 1.0f, 0.0f },
		{ -0.3f, -0.3f, -0.3f,  0.0f, 1.0f, 0.0f },

		// 2번 뒷면: 빨강, z = 0.3
		{ -0.3f,  0.3f,  0.3f,  1.0f, 0.0f, 0.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 0.0f },

		{ -0.3f,  0.3f,  0.3f,  1.0f, 0.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 0.0f },
		{  0.3f,  0.3f,  0.3f,  1.0f, 0.0f, 0.0f },

		// 3번 왼쪽 면: 파랑, x = -0.3
		{ -0.3f,  0.3f, -0.3f,  0.0f, 0.0f, 1.0f },
		{ -0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },
		{ -0.3f, -0.3f,  0.3f,  0.0f, 0.0f, 1.0f },

		{ -0.3f,  0.3f, -0.3f,  0.0f, 0.0f, 1.0f },
		{ -0.3f, -0.3f,  0.3f,  0.0f, 0.0f, 1.0f },
		{ -0.3f,  0.3f,  0.3f,  0.0f, 0.0f, 1.0f },

		// 4번 오른쪽 면: 노랑, x = 0.3
		{  0.3f,  0.3f, -0.3f,  1.0f, 1.0f, 0.0f },
		{  0.3f,  0.3f,  0.3f,  1.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 0.0f },

		{  0.3f,  0.3f, -0.3f,  1.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  1.0f, 1.0f, 0.0f },

		// 5번 윗면: 청록, y = 0.3
		{ -0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 1.0f },
		{ -0.3f,  0.3f,  0.3f,  0.0f, 1.0f, 1.0f },
		{  0.3f,  0.3f,  0.3f,  0.0f, 1.0f, 1.0f },

		{ -0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 1.0f },
		{  0.3f,  0.3f,  0.3f,  0.0f, 1.0f, 1.0f },
		{  0.3f,  0.3f, -0.3f,  0.0f, 1.0f, 1.0f },

		// 6번 아랫면: 자홍, y = -0.3
		{ -0.3f, -0.3f, -0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f, -0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 1.0f },

		{ -0.3f, -0.3f, -0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 1.0f },
		{ -0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 1.0f }
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
		// 7번 앞쪽 삼각형: 빨강
		// 꼭짓점, 오른쪽 아래, 왼쪽 아래
		{  0.0f,  0.4f,  0.0f,  1.0f, 0.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  1.0f, 0.0f, 0.0f },
		{ -0.3f, -0.3f, -0.3f,  1.0f, 0.0f, 0.0f },

		// 8번 왼쪽 삼각형: 파랑
		// 꼭짓점, 왼쪽 앞, 왼쪽 뒤
		{  0.0f,  0.4f,  0.0f,  0.0f, 0.0f, 1.0f },
		{ -0.3f, -0.3f, -0.3f,  0.0f, 0.0f, 1.0f },
		{ -0.3f, -0.3f,  0.3f,  0.0f, 0.0f, 1.0f },

		// 9번 뒤쪽 삼각형: 초록
		// 꼭짓점, 왼쪽 뒤, 오른쪽 뒤
		{  0.0f,  0.4f,  0.0f,  0.0f, 1.0f, 0.0f },
		{ -0.3f, -0.3f,  0.3f,  0.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  0.0f, 1.0f, 0.0f },

		// 0번 오른쪽 삼각형: 노랑
		// 꼭짓점, 오른쪽 뒤, 오른쪽 앞
		{  0.0f,  0.4f,  0.0f,  1.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 1.0f, 0.0f },
		{  0.3f, -0.3f, -0.3f,  1.0f, 1.0f, 0.0f },

		// 바닥면 첫 번째 삼각형: 자홍
		{ -0.3f, -0.3f, -0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f, -0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 1.0f },

		// 바닥면 두 번째 삼각형: 자홍
		{ -0.3f, -0.3f, -0.3f,  1.0f, 0.0f, 1.0f },
		{  0.3f, -0.3f,  0.3f,  1.0f, 0.0f, 1.0f },
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
	//애니메이션.
}

void DrawScene()
{
	//흰색 바탕.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);
	
	//단위 행렬로 저장.
	glm::mat4 baseTransform(1.0f);

	//변환 행렬.
	baseTransform = glm::rotate(
		baseTransform,
		glm::radians(-30.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	baseTransform = glm::rotate(
		baseTransform,
		glm::radians(-30.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	GLuint baseLocation = glGetUniformLocation(shaderProgram, "modelTransform");
	glUniformMatrix4fv(
		baseLocation,					//uniform변수의 위치를 정수로 변환한 값.
		1,								//넘길 행렬의 수.
		GL_FALSE,						//전치 할건지 안할건지.
		glm::value_ptr(baseTransform)	//변환 행렬의 첫번쨰 주소.
	);

	// 과제에 맞는 VAO와 그리기 함수를 이곳에서 호출한다.
	DrawCoordinateScene();
	DrawCubeScene();
	DrawPyramidScene();
	glUseProgram(0);
}

void DrawCoordinateScene()
{
	glBindVertexArray(coordinateVAO);

	glDrawArrays(GL_LINES, 0, 6);

	glBindVertexArray(0);
}

void DrawCubeScene()
{
	glBindVertexArray(cubeVAO);
	
	for (int i = 0; i < 6; ++i)
	{
		if (drawCubeFace[i]) {
			glDrawArrays(
				GL_TRIANGLES,
				i * 6,
				6
			);
		}
	}

	glBindVertexArray(0);
}

void DrawPyramidScene()
{
	glBindVertexArray(pyramidVAO);

	for (int i = 0; i < 4; ++i)
	{
		if (drawPyramidFace[i]) {
			glDrawArrays(
				GL_TRIANGLES,
				i * 3,
				3
			);
		}
	}

	if (drawPyramidBottom) {
		glDrawArrays(
			GL_TRIANGLES,
			12,
			6
		);
	}
	glBindVertexArray(0);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;
	(void)mods;

	if (action != GLFW_PRESS)
		return;

	//육면체 그리는 토글키.
	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6)
	{
		for (int i = 0; i < 4; ++i)
			drawPyramidFace[i] = false;
		drawPyramidBottom = false;

		int faceIndex = key - GLFW_KEY_1;
		drawCubeFace[faceIndex] = !drawCubeFace[faceIndex];
	}

	//피라미드 4면 그리는 토글키.
	if (key == GLFW_KEY_0)
	{
		for (int i = 0; i < 6; ++i)
			drawCubeFace[i] = false;
		drawPyramidBottom = false;
		drawPyramidFace[3] = !drawPyramidFace[3];
	}

	if (key >= GLFW_KEY_7 && key <= GLFW_KEY_9)
	{
		for (int i = 0; i < 6; ++i)
			drawCubeFace[i] = false;
		drawPyramidBottom = false;

		int faceIndex = key - GLFW_KEY_7;
		drawPyramidFace[faceIndex] = !drawPyramidFace[faceIndex];
	}

	if (key == GLFW_KEY_C)
	{
		for (int i = 0; i < 4; ++i)
			drawPyramidFace[i] = false;
		drawPyramidBottom = false;

		for (int i = 0; i < 6; ++i)
		{
			drawCubeFace[i] = false;
		}

		int firstFace = rand() % 6;

		int secondFace = 0;
		do
		{
			secondFace = rand() % 6;
		}
		while (firstFace == secondFace);

		drawCubeFace[firstFace] = true;
		drawCubeFace[secondFace] = true;
	}

	if (key == GLFW_KEY_T)
	{
		for (int i = 0; i < 6; ++i)
			drawCubeFace[i] = false;

		//나머지 면 전부 비활성화.
		for (int i = 0; i < 4; ++i)
		{
			drawPyramidFace[i] = false;
		}
		//피라미드의 제일 밑면 활성화.
		drawPyramidBottom = true;

		int randFace = rand() % 4;

		drawPyramidFace[randFace] = true;
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
