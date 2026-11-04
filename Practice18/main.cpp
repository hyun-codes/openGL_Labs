#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// 카메라 위치와 시선 및 원근 투영 범위를 저장한다.
struct Camera
{
	glm::vec3 position{0.0f, 3.2f, 9.0f};
	glm::vec3 target{ 0.0f, 0.0f, 0.0f };	//타겟은 원점.
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	float fovDegrees = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
};

// 행성의 궤도와 크기 및 색상과 그 주위를 도는 달의 상태를 함께 저장한다.
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

// 셰이더 프로그램과 재사용할 GPU 메시 및 장면 상태를 관리한다.
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

// 깊이 테스트와 표시 방식 및 키별 애니메이션 토글을 관리한다.
bool depthTestEnabled = true;
bool isWireFace = false;
bool isPerspective = true;
bool isOrbitScaling = false;
bool isCenterAnimating = false;
bool isMoonOrbitRotating = false;
// 중심 구 변화와 달 궤도 회전의 진행 각도를 따로 저장한다.
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

// 윈도우와 OpenGL을 초기화하고 입력 처리와 화면 갱신을 반복한 뒤 자원을 정리한다.
int main()
{
	// GLFW를 초기화하고 실패하면 프로그램 시작을 중단한다.
	if (!glfwInit()) return -1;
	// OpenGL 버전과 Core 프로파일을 지정한다.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 지정한 해상도의 윈도우와 OpenGL 컨텍스트를 생성한다.
	GLFWwindow* window = glfwCreateWindow(800, 800, "Practice18", nullptr, nullptr);
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
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
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
	// 종료 요청 전까지 상태 갱신과 렌더링 및 이벤트 처리를 반복한다.
	while (!glfwWindowShouldClose(window))
	{
		Update();
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

// OBJ의 위치와 면 인덱스를 읽고 삼각형 정점 배열로 바꾼다.
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

		// 위치 행은 원본 정점 좌표 목록에 저장한다.
		if (type == "v")
		{
			glm::vec3 position;
			if (stream >> position.x >> position.y >> position.z)
				positions.push_back(position);
		}
		// 면 행은 인덱스로 좌표를 찾아 실제로 그릴 정점으로 펼친다.
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

// 구의 OBJ 정점을 읽어 구 버퍼를 만들고 공전 궤도 버퍼를 준비한다.
bool InitBuffer()
{
	std::vector<Vertex> sphereVertices;
	if (!LoadObjVertices("models/sphere.obj", sphereVertices)) 
		return false;
	// 모든 행성과 달이 재사용할 구 정점을 GPU 버퍼에 저장한다.
	InitMeshBuffer(sphereMesh, sphereVertices);
	if (sphereMesh.VAO == 0) 
		return false;

	InitOrbitBuffer();

	return true;
}

// xz 평면의 반지름 1인 원을 만들고 끊기지 않는 선 고리로 저장한다.
void InitOrbitBuffer()
{
	std::vector<Vertex> orbitPosition;
	int segments = 96;						

	for (int i = 0; i < segments; ++i) {
		// 원주를 같은 각도 간격으로 나누어 궤도 정점 위치를 계산한다.
		float theta = 2.0f * PI * float(i) / segments;
		// 반지름 1인 원을 만들고, 그릴 때 행성/달의 궤도 반지름을 적용한다.
		float x = std::cos(theta);
		float z = std::sin(theta);
		orbitPosition.push_back({ x, 0, z, 1.0, 1.0f, 1.0f });
	}	
	InitMeshBuffer(orbitMesh, orbitPosition, GL_LINE_LOOP);
}

// 객체와 카메라 및 애니메이션 상태를 처음 값으로 되돌린다.
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

// WASD와 더하기 및 빼기 키로 카메라가 아닌 전체 장면의 위치를 이동한다.
void ProcessInput(float deltaTime)
{
	// 여러 키의 이동을 합치고 정규화해 대각선 이동도 같은 속력으로 처리한다.
	glm::vec3 movement(0.0f);
	if (glfwGetKey(mainWindow, GLFW_KEY_W) == GLFW_PRESS) movement.y += 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_S) == GLFW_PRESS) movement.y -= 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_A) == GLFW_PRESS) movement.x -= 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_D) == GLFW_PRESS) movement.x += 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_KP_ADD) == GLFW_PRESS) movement.z += 1.0f;
	if (glfwGetKey(mainWindow, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(mainWindow, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) movement.z -= 1.0f;
	if (glm::length(movement) > 0.0f) scenePosition += glm::normalize(movement) * 1.2f * deltaTime;
}

// 실제 경과 시간으로 행성과 달의 공전 및 선택한 중심 구와 달 궤도 애니메이션을 갱신한다.
void Update()
{
	double currentTime = glfwGetTime();
	float deltaTime = static_cast<float>(currentTime - lastFrameTime);
	lastFrameTime = currentTime;
	// 긴 프레임에서 이동이 과도하게 건너뛰지 않도록 경과 시간을 제한한다.
	deltaTime = std::clamp(deltaTime, 0.0f, 0.05f);
	ProcessInput(deltaTime);
	// 세 행성과 각 달의 각도를 서로 다른 공전 속도로 계속 갱신한다.
	for (Planet& planet : planets)
	{
		planet.orbitAngle = std::fmod(planet.orbitAngle + planet.orbitSpeed * deltaTime, 360.0f);
		planet.moonAngle = std::fmod(planet.moonAngle + planet.moonSpeed * deltaTime, 360.0f);
	}
	// 중심 구 애니메이션이 켜진 동안에만 크기와 색상 변화 각도를 진행한다.
	if (isCenterAnimating) centerAnimationAngle = std::fmod(centerAnimationAngle + 70.0f * deltaTime, 360.0f);
	// 달 궤도 회전이 켜진 동안에만 궤도면의 추가 회전 각도를 진행한다.
	if (isMoonOrbitRotating) moonOrbitRotateAngle = std::fmod(moonOrbitRotateAngle + 40.0f * deltaTime, 360.0f);
}

// 투영과 공통 장면 변환을 설정하고 중심 구와 행성 및 달과 모든 궤도를 그린다.
void DrawScene()
{
	glClearColor(0.025f, 0.025f, 0.07f, 1.0f);					//배경색.
	// 색상과 깊이 버퍼를 지워 이전 프레임의 결과를 제거한다.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 이후 그리기에 사용할 셰이더 프로그램을 활성화한다.
	glUseProgram(shaderProgram);

	// 와이어 모드일 때 삼각형의 내부 대신 테두리를 그린다.
	if (isWireFace)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;

	glm::mat4 view = glm::lookAt(camera.position, camera.target, camera.up);
	glm::mat4 projection;
	// 원근 투영과 직각 투영 중 선택된 투영 행렬을 만든다.
	if (isPerspective)
		projection = glm::perspective(glm::radians(camera.fovDegrees), aspect, camera.nearPlane, camera.farPlane);
	else
	{
		float halfHeight = glm::length(camera.position - camera.target) * std::tan(glm::radians(camera.fovDegrees) * 0.5f);
		projection = glm::ortho(-halfHeight * aspect, halfHeight * aspect, -halfHeight, halfHeight, camera.nearPlane, camera.farPlane);
	}
	// 모든 객체와 궤도가 공유하는 전체 장면 이동 행렬을 만든다.
	glm::mat4 sceneTransform = glm::translate(glm::mat4(1.0f), scenePosition);
	// 궤도 확대 토글에 따라 반지름을 원래 크기 또는 1.25배로 선택한다.
	float orbitScale = isOrbitScaling ? 1.25f : 1.0f;
	// 진행 각도의 사인 값으로 중심 구 반지름을 일정 범위 안에서 바꾼다.
	float centerRadius = 0.45f + 0.15f * std::sin(glm::radians(centerAnimationAngle));
	// 코사인 값으로 두 색의 보간 비율을 만들어 급격한 색상 변화를 피한다.
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
		// 행성마다 다른 기울기를 적용하여 서로 다른 공전 평면을 만든다.
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
		// 행성과 달의 반지름에 같은 확대 배율을 적용하여 궤도선과 실제 위치를 일치시킨다.
		float moonOrbitRadius = planet.moonOrbitRadius * orbitScale;
		DrawOrbitScene(glm::scale(moonFrame, glm::vec3(moonOrbitRadius)), glm::vec3(0.7f));

		float moonAngle = glm::radians(planet.moonAngle);
		glm::vec3 moonPosition(moonOrbitRadius * std::cos(moonAngle), 0.0f,
			moonOrbitRadius * std::sin(moonAngle));
		glm::mat4 moonTransform = glm::translate(moonFrame, moonPosition);
		DrawSphereScene(glm::scale(moonTransform, glm::vec3(planet.moonRadius)), planet.moonColor);
	}
}

// 구의 모델 행렬과 색상을 셰이더에 전달하고 같은 구 버퍼를 재사용한다.
void DrawSphereScene(const glm::mat4& modelTransform, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, glm::value_ptr(modelTransform));
	glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(color));
	DrawMesh(sphereMesh);
}

// 궤도별 위치와 회전 및 반지름을 모델 행렬로 전달하고 선 고리를 그린다.
void DrawOrbitScene(const glm::mat4& orbitTransform, const glm::vec3& color)
{
	GLint modelLocation = glGetUniformLocation(shaderProgram, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(orbitTransform));

	GLint modelColor = glGetUniformLocation(shaderProgram, "objectColor");
	glUniform3fv(modelColor, 1, glm::value_ptr(color));
	DrawMesh(orbitMesh);
}

// 투영과 와이어 및 궤도 확대와 애니메이션 토글을 키를 누를 때 한 번 처리한다.
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)scancode;

	if (action != GLFW_PRESS)
		return;

	// 깊이 테스트 사용 여부를 전환한다.
	if (key == GLFW_KEY_H) {
		depthTestEnabled = !depthTestEnabled;

		if (depthTestEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}
	// 소문자 p는 직각 투영이고 Shift와 p는 원근 투영이다.
	if (key == GLFW_KEY_P) isPerspective = (mods & GLFW_MOD_SHIFT) != 0;
	// 첫 번째 키는 궤도 반지름을 확대하거나 원래 크기로 되돌린다.
	if (key == GLFW_KEY_1 || key == GLFW_KEY_KP_1) isOrbitScaling = !isOrbitScaling;
	// 두 번째 키는 중심 구 크기와 색상 변화의 시작과 정지를 전환한다.
	if (key == GLFW_KEY_2 || key == GLFW_KEY_KP_2) isCenterAnimating = !isCenterAnimating;
	// 세 번째 키는 달 궤도면 회전의 시작과 정지를 전환한다.
	if (key == GLFW_KEY_3 || key == GLFW_KEY_KP_3) isMoonOrbitRotating = !isMoonOrbitRotating;

	// 소문자 m은 솔리드이고 Shift와 m은 와이어 표시를 선택한다.
	if (key == GLFW_KEY_M) 
		isWireFace = (mods & GLFW_MOD_SHIFT) != 0;
	// 장면과 투영 및 모든 애니메이션 토글을 초기화한다.
	if (key == GLFW_KEY_R) ResetScene();

	// 종료 요청을 윈도우에 전달한다.
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
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
	DeleteMesh(sphereMesh);
	DeleteMesh(orbitMesh);
	glDeleteProgram(shaderProgram);
}
