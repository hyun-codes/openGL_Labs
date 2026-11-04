#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// 정점 하나의 위치와 색상을 저장한다.
struct Vertex
{
	GLfloat x, y, z;
	GLfloat R, G, B;
};

// VAO와 VBO 및 그릴 정점 개수를 저장한다.
struct Mesh
{
	GLuint VAO = 0;
	GLuint VBO = 0;
	GLsizei vertexCount = 0;
	GLenum drawMode = GL_TRIANGLES;
};

// 셰이더 파일을 문자열로 읽는다.
inline std::string ReadShaderFile(const char* filePath)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "셰이더 파일을 열 수 없습니다: " << filePath << std::endl;
		return {};
	}
	std::ostringstream stream;
	stream << file.rdbuf();
	return stream.str();
}

// 셰이더를 컴파일하고 실패하면 오류를 출력한다.
inline GLuint CompileShader(GLenum shaderType, const char* filePath)
{
	std::string source = ReadShaderFile(filePath);
	if (source.empty()) return 0;

	GLuint shader = glCreateShader(shaderType);
	const char* sourceText = source.c_str();
	glShaderSource(shader, 1, &sourceText, nullptr);
	glCompileShader(shader);

	GLint success = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE)
	{
		GLchar errorLog[1024] = {};
		glGetShaderInfoLog(shader, sizeof(errorLog), nullptr, errorLog);
		std::cerr << filePath << " 컴파일 실패: " << errorLog << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

// 정점 셰이더와 프래그먼트 셰이더를 연결한다.
inline GLuint CreateShaderProgram(const char* vertexPath, const char* fragmentPath)
{
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexPath);
	if (vertexShader == 0) return 0;
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentPath);
	if (fragmentShader == 0)
	{
		glDeleteShader(vertexShader);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint success = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success != GL_TRUE)
	{
		GLchar errorLog[1024] = {};
		glGetProgramInfoLog(program, sizeof(errorLog), nullptr, errorLog);
		std::cerr << "셰이더 링크 실패: " << errorLog << std::endl;
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

// 정점을 GPU에 저장하고 위치와 색상 속성을 VAO에 설정한다.
inline void InitMeshBuffer(Mesh& mesh, const std::vector<Vertex>& vertices, GLenum drawMode = GL_TRIANGLES)
{
	if (vertices.empty()) 
		return;
	mesh.vertexCount = static_cast<GLsizei>(vertices.size());
	mesh.drawMode = drawMode;

	glGenVertexArrays(1, &mesh.VAO);
	glBindVertexArray(mesh.VAO);

	glGenBuffers(1, &mesh.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

// VAO를 연결하고 저장된 정점 개수만큼 그린다.
inline void DrawMesh(const Mesh& mesh)
{
	if (mesh.VAO == 0) return;
	glBindVertexArray(mesh.VAO);
	glDrawArrays(mesh.drawMode, 0, mesh.vertexCount);
	glBindVertexArray(0);
}

// GPU에 생성한 정점 버퍼와 VAO를 해제한다.
inline void DeleteMesh(Mesh& mesh)
{
	if (mesh.VBO != 0) glDeleteBuffers(1, &mesh.VBO);
	if (mesh.VAO != 0) glDeleteVertexArrays(1, &mesh.VAO);
	mesh = {};
}

// 도형과 카메라 및 투영 행렬을 셰이더로 전달한다.
inline void SetTransformUniforms(GLuint program, const glm::mat4& model,
	const glm::mat4& view, const glm::mat4& projection)
{
	glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(program, "viewTransform"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(program, "projectionTransform"), 1, GL_FALSE, glm::value_ptr(projection));
}
