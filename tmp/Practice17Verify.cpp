#include "../PracticeStarterGL.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <array>

std::array<int, GLFW_KEY_LAST + 1> testKeys{};
int TestGetKey(GLFWwindow*, int key) { return testKeys[key]; }
#define glfwGetKey TestGetKey
#define main Practice17Main
#include "../Practice17/main.cpp"
#undef main
#undef glfwGetKey

bool Close(const glm::vec3& a, const glm::vec3& b) { return glm::length(a - b) < 0.0005f; }
bool Close(float a, float b) { return std::abs(a - b) < 0.0005f; }
bool Close(const glm::mat4& a, const glm::mat4& b)
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			if (!Close(a[i][j], b[i][j])) return false;
	return true;
}

void TestTransforms()
{
	ObjectState object;
	object.position = glm::vec3(1.0f, 0.0f, 0.0f);
	RotateObject(object, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	assert(Close(object.position, glm::vec3(1.0f, 0.0f, 0.0f)));
	ScaleObject(object, 2.0f, false);
	assert(Close(object.position, glm::vec3(1.0f, 0.0f, 0.0f)));
	ScaleObject(object, 0.5f, true);
	assert(Close(object.position, glm::vec3(0.5f, 0.0f, 0.0f)));
	OrbitObject(object, 90.0f);
	assert(Close(object.position, glm::vec3(0.0f, 0.0f, -0.5f)));
	assert(Close(glm::vec3(MakeObjectTransform(object, glm::mat4(1.0f)) * glm::vec4(0, 0, 0, 1)), object.position));
}

void TestSwapPaths()
{
	ObjectState state[2];
	state[0].position = glm::vec3(-1.0f, 0.5f, 0.2f);
	state[1].position = glm::vec3(2.0f, -0.4f, -0.3f);
	glm::vec3 start0 = state[0].position, start1 = state[1].position;
	SwapAnimation animation;
	StartSwap(animation, state, false);
	UpdateSwap(animation, state, 1.0f);
	assert(Close(state[0].position, glm::vec3(0.0f)) && Close(state[1].position, glm::vec3(0.0f)));
	UpdateSwap(animation, state, 1.0f);
	assert(!animation.active && Close(state[0].position, start1) && Close(state[1].position, start0));
	StartSwap(animation, state, true);
	UpdateSwap(animation, state, 1.0f);
	assert(state[0].position.y > state[1].position.y);
	UpdateSwap(animation, state, 1.0f);
	assert(Close(state[0].position, start0) && Close(state[1].position, start1));
	StartSwap(animation, state, false);
	UpdateSwap(animation, state, 0.5f);
	glm::vec3 before = state[0].position;
	StartSwap(animation, state, true);
	UpdateSwap(animation, state, 0.0f);
	assert(Close(before, state[0].position));
	state[0].position.x += 0.3f;
	glm::vec3 expected = animation.start[1] + glm::vec3(0.3f, 0.0f, 0.0f);
	UpdateSwap(animation, state, 2.0f);
	assert(Close(expected, state[0].position));
}

void TestKeyboard()
{
	ResetScene();
	keyCallBack(mainWindow, GLFW_KEY_1, 0, GLFW_PRESS, 0);
	assert(selectedObject == 0);
	ObjectState right = objects[1];
	for (int key : {GLFW_KEY_X, GLFW_KEY_Y, GLFW_KEY_R, GLFW_KEY_A, GLFW_KEY_B, GLFW_KEY_D, GLFW_KEY_E})
	{
		ObjectState before = objects[0];
		testKeys[key] = GLFW_PRESS;
		ProcessTransformInput(0.25f);
		assert(!Close(objects[0].position, before.position) || !Close(objects[0].rotation, before.rotation) || !Close(objects[0].scale, before.scale));
		testKeys[GLFW_KEY_LEFT_SHIFT] = GLFW_PRESS;
		ProcessTransformInput(0.25f);
		assert(Close(objects[0].position, before.position) && Close(objects[0].rotation, before.rotation) && Close(objects[0].scale, before.scale));
		testKeys.fill(0);
	}
	assert(Close(objects[1].position, right.position) && Close(objects[1].rotation, right.rotation));
	keyCallBack(mainWindow, GLFW_KEY_2, 0, GLFW_PRESS, 0);
	assert(selectedObject == 1);
	keyCallBack(mainWindow, GLFW_KEY_3, 0, GLFW_PRESS, 0);
	assert(selectedObject == 2);
	keyCallBack(mainWindow, GLFW_KEY_V, 0, GLFW_PRESS, 0);
	assert(combinedAnimation);
	keyCallBack(mainWindow, GLFW_KEY_V, 0, GLFW_RELEASE, 0);
	assert(combinedAnimation);
	Update(0.01f);
	assert(objects[0].scale > 0.55f && objects[1].scale < 0.55f);
	for (int i = 0; i < 3000; ++i) Update(0.016f);
	for (const auto& object : objects) assert(object.scale >= minimumScale && object.scale <= maximumScale);
	glm::vec3 currentPosition = objects[0].position;
	int oldModel = objects[0].modelIndex;
	keyCallBack(mainWindow, GLFW_KEY_C, 0, GLFW_PRESS, 0);
	assert(oldModel != objects[0].modelIndex && Close(currentPosition, objects[0].position));
	assert(modelNames[objects[0].modelIndex] == "cone" && modelNames[objects[1].modelIndex] == "torus");
	DrawScene();
	assert(glGetError() == GL_NO_ERROR);
	keyCallBack(mainWindow, GLFW_KEY_C, 0, GLFW_PRESS, 0);
	assert(modelNames[objects[0].modelIndex] == "cone" && modelNames[objects[1].modelIndex] == "torus");
	keyCallBack(mainWindow, GLFW_KEY_T, 0, GLFW_PRESS, 0);
	assert(swapAnimation.active && !swapAnimation.curved);
	keyCallBack(mainWindow, GLFW_KEY_U, 0, GLFW_PRESS, 0);
	assert(swapAnimation.active && swapAnimation.curved);
	keyCallBack(mainWindow, GLFW_KEY_S, 0, GLFW_PRESS, 0);
	assert(!swapAnimation.active && !combinedAnimation && selectedObject == 2);
	assert(Close(objects[0].position, glm::vec3(-0.9f, 0.0f, 0.0f)));
	assert(modelNames[objects[0].modelIndex] == "sphere" && modelNames[objects[1].modelIndex] == "monkey");
}

int main()
{
	TestTransforms();
	TestSwapPaths();
	assert(glfwInit());
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	mainWindow = glfwCreateWindow(400, 400, "Practice17 verification", nullptr, nullptr);
	assert(mainWindow);
	glfwMakeContextCurrent(mainWindow);
	glewExperimental = GL_TRUE;
	assert(glewInit() == GLEW_OK);
	while (glGetError() != GL_NO_ERROR) {}
	framebufferWidth = framebufferHeight = 400;
	glViewport(0, 0, 400, 400);
	glEnable(GL_DEPTH_TEST);
	shaderProgram = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	assert(shaderProgram && InitBuffer());
	TestKeyboard();
	DrawScene();
	assert(glGetError() == GL_NO_ERROR);
	std::vector<unsigned char> pixels(400 * 400 * 3);
	glReadPixels(0, 0, 400, 400, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	int nonBackground = 0;
	for (int i = 0; i < 400 * 400; ++i)
		if (pixels[i * 3] > 50 || pixels[i * 3 + 1] > 50 || pixels[i * 3 + 2] > 50) ++nonBackground;
	assert(nonBackground > 3000);
	std::ofstream preview("../tmp/Practice17-preview.ppm", std::ios::binary);
	preview << "P6\n400 400\n255\n";
	for (int y = 399; y >= 0; --y) preview.write(reinterpret_cast<char*>(pixels.data() + y * 400 * 3), 400 * 3);
	preview.close();
	keyCallBack(mainWindow, GLFW_KEY_Q, 0, GLFW_PRESS, 0);
	assert(glfwWindowShouldClose(mainWindow));
	std::cout << "PASS: local/origin transforms, all keys, swap continuity, scale bounds, OBJ loading, shader linking and GPU rendering. Pixels=" << nonBackground << std::endl;
	Cleanup();
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
