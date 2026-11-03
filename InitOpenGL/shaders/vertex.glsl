#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;

uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;

out vec3 vertexColor;

void main()
{
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPosition, 1.0);
    vertexColor = vColor;
}
