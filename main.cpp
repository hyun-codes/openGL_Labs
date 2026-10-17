#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<random>


#include <iostream>

void InputProcess(GLFWwindow* window);
void DrawScene();
void SetRandomColor();
bool isRunning = false;
double lastColorTime = 0.0f;

int main() {
	if (!glfwInit()) {
		std::cerr << "GLFW 초기화 실패!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//--- 윈도우 생성
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
	if (!window) {
		std::cerr << "윈도우 생성 실패!" << std::endl;
		glfwTerminate();
		return -1;
	}
	//--- 컨텍스트 설정
	glfwMakeContextCurrent(window);

	//--- GLEW 초기화
	glewExperimental = GL_TRUE; // 최신 기능 사용
	if (glewInit() != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패!" << std::endl;
		return -1;
	}
	//--- 뷰포트 설정
	glViewport(0, 0, 800, 600);

	//--- 메인 루프
	while (!glfwWindowShouldClose(window)) {
		
		// 키보드 입력 처리
		InputProcess(window);

		double currentTIme = glfwGetTime();
		//k키가 눌러져있는 경우.
		if (isRunning)
		{
			if (currentTIme - lastColorTime >= 1.0f)
			{
				SetRandomColor();
				lastColorTime = currentTIme;
			}
		}

		// 화면 지우기 (파란색)
		DrawScene();

		// 버퍼 교체
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//--- 종료 처리
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void InputProcess(GLFWwindow* window) {
	std::random_device rd{};
	std::mt19937 gen(rd());

	std::uniform_real_distribution<double> dis(0.0f, 1.0f);

	float red = dis(gen);
	float green = dis(gen);
	float blue = dis(gen);

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)		//C키 입력.
		glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)		//M키 입력.
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)		//Y키 입력.
		glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
		
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)		//A키 입력.
		glClearColor(red, green, blue, 1.0f);
			
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)		//G키 입력.
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)		//K키 입력.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)		//T값 누르기.
	{
		 isRunning = true;
		 lastColorTime = glfwGetTime();
	}
	
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		isRunning = false;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)	//esc 
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	glClear(GL_COLOR_BUFFER_BIT);
}

void DrawScene() {

}

void SetRandomColor()
{
	std::random_device rd{};
	std::mt19937 gen(rd());

	std::uniform_real_distribution<double> dist(0.0, 1.0);

	double red = dist(gen);
	double green = dist(gen);
	double blue = dist(gen);

	glClearColor(red, green, blue, 1.0f);
}