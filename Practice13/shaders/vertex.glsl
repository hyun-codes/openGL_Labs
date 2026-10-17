#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;

uniform mat4 modelTransform;

out vec3 vertexColor;

void main()
{
    //변환 행렬을 넘겨주는 코드.
    gl_Position = modelTransform * vec4(vPosition, 1.0);
    vertexColor = vColor;
}
