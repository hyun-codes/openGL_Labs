#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
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
void DrawCubeScene();
void DrawPyramidScene();
void Cleanup();

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferCallBack(GLFWwindow* window, int width, int height);
//현재 회전각도. 
float rotateAngleX = 0.0f;
float rotateAngleY = 0.0f;
//초당 회전량.
float rotateVelocityX = 0.0f;
float rotateVelocityY = 0.0f;
//+인지 -인지에 따른 초기 세팅.
const float rotateSpeed = 60.0f;
//이동 변수.
float moveX = 0.0f;
float moveY = 0.0f;

double lastFrameTime = 0.0f;

bool drawCube = false;
bool drawPyramid = false;
bool depthTestEnabled = true;

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
	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice14", nullptr, nullptr);
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
	//전 프레임 시간.
	lastFrameTime = glfwGetTime();

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
	//deltaTime 계산.
	double currentTime = glfwGetTime();
	float deltaTime = static_cast<float>(currentTime - lastFrameTime);
	lastFrameTime = currentTime;

	//만약 60도이고 프레임이 0.1이라면 +6도만큼 회전.
	rotateAngleX += rotateVelocityX * deltaTime;
	rotateAngleY += rotateVelocityY * deltaTime;
}

void DrawScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
		baseLocation,
		1,
		GL_FALSE,
		glm::value_ptr(baseTransform)
	);
	//좌표축 그리기.
	DrawCoordinateScene();

	//좌표축의 기본 회전 안에서 이동한다.
	glm::mat4 objectTransform = baseTransform;
	objectTransform = glm::translate(
		objectTransform,
		glm::vec3(moveX, moveY, 0.0f)
	);

	objectTransform = glm::rotate(
		objectTransform,
		glm::radians(rotateAngleX),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	objectTransform = glm::rotate(
		objectTransform,
		glm::radians(rotateAngleY),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	GLuint objectLocation = glGetUniformLocation(shaderProgram, "modelTransform");
	glUniformMatrix4fv(
		objectLocation,
		1,
		GL_FALSE,
		glm::value_ptr(objectTransform)
	);

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

	if (drawCube)
		glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
}

void DrawPyramidScene()
{
	glBindVertexArray(pyramidVAO);

	if(drawPyramid)
		glDrawArrays(GL_TRIANGLES, 0, 18);

	glBindVertexArray(0);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;

	if (action != GLFW_PRESS)
		return;
	//육면체 그리기.
	if (key == GLFW_KEY_C)
	{
		drawCube = true;
		drawPyramid = false;
	}
	//사각뿔 그리기.
	if (key == GLFW_KEY_P)
	{
		drawCube = false;
		drawPyramid = true;
	}
	//은면제거 적용/해제.
	if (key == GLFW_KEY_H){
		depthTestEnabled = !depthTestEnabled;

		if (depthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
	//와이어 객체 솔리드 객체.
	if (key == GLFW_KEY_W){		
		if (mods & GLFW_MOD_SHIFT) 	//보조키.
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	//x축방향으로 회전.
	if (key == GLFW_KEY_X) {
		if (mods & GLFW_MOD_SHIFT)
			rotateVelocityX = -rotateSpeed;
		else
			rotateVelocityX = rotateSpeed;
	}
	//y축방향으로 회전.
	if (key == GLFW_KEY_Y) {
		if (mods & GLFW_MOD_SHIFT)
			rotateVelocityY = -rotateSpeed;
		else
			rotateVelocityY = rotateSpeed;
	}

	//이동 키 구현.
	if (key == GLFW_KEY_RIGHT){
		moveX += 0.1f;
	}
		
	if (key == GLFW_KEY_LEFT) {
		moveX -= 0.1f;
	}
		
	if (key == GLFW_KEY_UP) {
		moveY += 0.1f;
	}
		
	if (key == GLFW_KEY_DOWN) {
		moveY -= 0.1f;

	}
		

	//애니메이션 정지.
	if (key == GLFW_KEY_S) {
		moveX = 0.0f;
		moveY = 0.0f;

		rotateAngleX = 0.0f;
		rotateAngleY = 0.0f;

		rotateVelocityX = 0.0f;
		rotateVelocityY = 0.0f;
	}

	//프로그램 종료.
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
