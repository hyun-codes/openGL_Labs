#version 330 core
layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec3 vNormal;

uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;
out vec3 worldNormal;

void main()
{
    worldNormal = transpose(inverse(mat3(modelTransform))) * vNormal;
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPosition, 1.0);
}
