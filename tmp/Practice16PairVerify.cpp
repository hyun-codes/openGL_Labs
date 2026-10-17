#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <cassert>
#include <vector>
#define main Practice16Main
#include "../Practice16/main.cpp"
#undef main

PFNGLUNIFORMMATRIX4FVPROC savedUniform;
std::vector<glm::mat4> capturedTransforms;
void APIENTRY CaptureUniform(GLint location, GLsizei count, GLboolean transpose, const GLfloat* values)
{
	capturedTransforms.push_back(glm::make_mat4(values));
	savedUniform(location, count, transpose, values);
}

bool Close(const glm::mat4& a, const glm::mat4& b)
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			if (std::abs(a[i][j] - b[i][j]) > 0.0001f) return false;
	return true;
}

void Advance(float seconds)
{
	double now = glfwGetTime() + seconds;
	glfwSetTime(now);
	lastFrameTime = now - seconds;
	Update();
}

void CheckDraw(GLint location)
{
	capturedTransforms.clear();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	__glewUniformMatrix4fv = CaptureUniform;
	DrawPyramidScene(glm::mat4(1.0f), location);
	__glewUniformMatrix4fv = savedUniform;
	assert(capturedTransforms.size() == 5);
	for (int face = 0; face < 4; ++face)
	{
		glm::mat4 expected = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.4f, 0.0f));
		expected = glm::rotate(expected, glm::radians(oppositePyramidFacesSwapAngleY[face % 2]), glm::vec3(0.0f, 1.0f, 0.0f));
		expected = glm::translate(expected, glm::vec3(0.0f, -0.4f, 0.0f));
		assert(Close(capturedTransforms[face], expected));
	}
	assert(Close(capturedTransforms[4], glm::mat4(1.0f)));
	assert(glGetError() == GL_NO_ERROR);
}

int main()
{
	assert(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(300, 300, "Verify pair swap", nullptr, nullptr);
	assert(window);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	assert(glewInit() == GLEW_OK);
	while (glGetError() != GL_NO_ERROR) {}
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	assert(shaderProgram);
	InitBuffer();
	glUseProgram(shaderProgram);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 300, 300);
	savedUniform = __glewUniformMatrix4fv;
	GLint location = glGetUniformLocation(shaderProgram, "modelTransform");
	drawCube = false;
	pyramidSwapQueue = {0, 1, 0};
	Advance(1.0f);
	assert(selectedOppositePyramidPair == 0 && std::abs(oppositePyramidFacesSwapAngleY[0] - 60.0f) < 0.01f && oppositePyramidFacesSwapAngleY[1] == 0.0f);
	CheckDraw(location);
	Advance(2.0f);
	assert(!isOppositePyramidFacesSwapping && oppositePyramidFacesSwapAngleY[0] == 180.0f);
	Advance(1.0f);
	assert(selectedOppositePyramidPair == 1 && oppositePyramidFacesSwapAngleY[0] == 180.0f && std::abs(oppositePyramidFacesSwapAngleY[1] - 60.0f) < 0.01f);
	CheckDraw(location);
	Advance(2.0f);
	Advance(3.0f);
	assert(oppositePyramidFacesSwapAngleY[0] == 0.0f && oppositePyramidFacesSwapAngleY[1] == 180.0f);
	CheckDraw(location);
	std::srand(17);
	std::vector<int> expected;
	for (int i = 0; i < 20; ++i) expected.push_back(std::rand() % 2);
	std::srand(17);
	for (int i = 0; i < 20; ++i) keyCallBack(window, GLFW_KEY_8, 0, GLFW_PRESS, 0);
	assert(pyramidSwapQueue.size() == 20);
	for (int i = 0; i < 20; ++i) assert(pyramidSwapQueue[i] == expected[i]);
	keyCallBack(window, GLFW_KEY_8, 0, GLFW_RELEASE, 0);
	assert(pyramidSwapQueue.size() == 20);
	keyCallBack(window, GLFW_KEY_R, 0, GLFW_PRESS, 0);
	assert(pyramidSwapQueue.empty() && !isOppositePyramidFacesSwapping);
	assert(oppositePyramidFacesSwapAngleY[0] == 0.0f && oppositePyramidFacesSwapAngleY[1] == 0.0f);
	keyCallBack(window, GLFW_KEY_8, 0, GLFW_PRESS, 0);
	assert(pyramidSwapQueue.empty());
	std::cout << "PASS: random pair input, queued presses, only selected pair moves, persistent swaps, fixed apex, unchanged bottom and reset." << std::endl;
	Cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
}
