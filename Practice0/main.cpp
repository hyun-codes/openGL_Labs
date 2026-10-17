#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

//--- 사용자 정의 함수
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
void InitBuffer();
void drawScene();
void Reshape(GLFWwindow* window, int width, int height);

//--- 셰이더 파일을 읽고 오류를 확인하기 위한 보조 함수
std::string readShaderFile(const char* filePath);
bool checkShaderCompile(GLuint shader, const char* shaderName);
bool checkProgramLink(GLuint program);

//--- 필요한 변수 선언
GLint width;
GLint height;
GLuint shaderProgramID = 0;
GLuint vertexShader = 0;
GLuint fragmentShader = 0;

//--- 앞으로 직접 생성하고 설정할 VAO, VBO
GLuint VAO = 0;
GLuint VBO = 0;
GLuint EBO = 0;

//--- 메인 함수
int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	width = 500;
	height = 500;

	//--- GLFW 초기화
	if (!glfwInit())
	{
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	//--- OpenGL 버전 및 프로파일 설정
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//--- 윈도우 생성
	GLFWwindow* window =
		glfwCreateWindow(width, height, "Example1", nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	//--- 생성한 윈도우의 OpenGL Context를 현재 Context로 설정
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, Reshape);

	//--- GLEW 초기화
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	//--- 셰이더 읽어와서 셰이더 프로그램 만들기
	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	if (shaderProgramID == 0)
	{
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	//--- VAO, VBO 연습을 시작할 자리
	InitBuffer();

	//--- 렌더링 루프
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		drawScene();
		//--- 윈도우에 렌더링 결과 출력
		glfwSwapBuffers(window);

		//--- 이벤트 처리
		glfwPollEvents();
	}

	//--- 종료
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteProgram(shaderProgramID);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

//vertax.glsl을 읽고 컴파일.
void make_vertexShaders()
{
	std::string vertexSource = readShaderFile("shaders/vertex.glsl");
	if (vertexSource.empty())
		return;

	const char* source = vertexSource.c_str();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);	//내부에서 버텍스 세이더 객체를 opengl내부에서 만들고, 그 객체를 가리키는 핸들러에 ID넣기.
	glShaderSource(vertexShader, 1, &source, nullptr);	//(핸들러, 문자열의 개수, 
	glCompileShader(vertexShader);
	checkShaderCompile(vertexShader, "vertex.glsl");
}

//fragment.glsl을 읽고 컴파일.
void make_fragmentShaders()
{
	std::string fragmentSource = readShaderFile("shaders/fragment.glsl");
	if (fragmentSource.empty())
		return;

	const char* source = fragmentSource.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &source, nullptr);
	glCompileShader(fragmentShader);
	checkShaderCompile(fragmentShader, "fragment.glsl");
}

//--- 셰이더 프로그램 만들기
GLuint make_shaderProgram()
{
	if (vertexShader == 0 || fragmentShader == 0)
		return 0;

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	if (!checkProgramLink(shaderProgram))
	{
		glDeleteProgram(shaderProgram);
		return 0;
	}

	return shaderProgram;	//셰이더 프로그램의 ID를 반환해준다.
}

//--- VAO, VBO 설정
void InitBuffer()
{
	// TODO 1: C++에 정점 좌표 배열 만들기
	float vPositonList[] = {
		0.5f, 0.5f, 0.0f,	// 우측 상단
		0.5f, -0.5f, 0.0f,	// 우측 하단
	    -0.5f, -0.5f, 0.0f, // 좌측 하단
	    -0.5f, 0.5f, 0.0f	// 좌측 상단
	};

	unsigned int index[] = {
		0, 1, 3,			//첫번쨰 삼각형.
		1, 2, 3				//두번째 삼각형.
	};

	float vcolors[3][3] = {
		// 색상
		{1.0f, 0.0f, 0.0f},   // 빨강
		{0.0f, 1.0f, 0.0f},   // 초록
		{0.0f, 0.0f, 1.0f}    // 파랑
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vPositonList), vPositonList, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

//--- 화면 그리기
void drawScene()
{
	//--- 배경만 지우는 기본 상태
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// TODO 6: 셰이더 프로그램 사용
	glUseProgram(shaderProgramID);
	// TODO 7: 그릴 VAO 바인딩
	glBindVertexArray(VAO);		//init에서 vbo의 그리는 방식을 저장하고 그 방식으로 그리겠다는 의미.
	// TODO 8: glDrawArrays()로 점, 선 또는 삼각형 그리기
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

//--- 윈도우 크기 변경
void Reshape(GLFWwindow* window, int newWidth, int newHeight)
{
	(void)window;
	width = newWidth;
	height = newHeight;
	glViewport(0, 0, width, height);
}

//--- 외부 GLSL 파일 읽기
std::string readShaderFile(const char* filePath)
{
	std::ifstream shaderFile(filePath);
	if (!shaderFile.is_open())
	{
		std::cerr << "셰이더 파일을 열 수 없습니다: "
			<< filePath << std::endl;
		return "";
	}

	std::stringstream shaderStream;
	shaderStream << shaderFile.rdbuf();
	return shaderStream.str();
}

//--- 셰이더 컴파일 확인
bool checkShaderCompile(GLuint shader, const char* shaderName)
{
	GLint success = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		char infoLog[1024] = {};
		glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
		std::cerr << shaderName << " 컴파일 실패!\n"
			<< infoLog << std::endl;
		return false;
	}

	return true;
}

//--- 셰이더 프로그램 링크 확인
bool checkProgramLink(GLuint program)
{
	GLint success = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		char infoLog[1024] = {};
		glGetProgramInfoLog(program, 1024, nullptr, infoLog);
		std::cerr << "셰이더 프로그램 링크 실패!\n"
			<< infoLog << std::endl;
		return false;
	}

	return true;
}
