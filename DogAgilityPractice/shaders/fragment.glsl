#version 330 core
in vec3 worldNormal;
out vec4 FragColor;
uniform vec3 objectColor;

void main()
{
    float brightness = 0.65 + 0.35 * max(dot(normalize(worldNormal), normalize(vec3(0.4, 1.0, 0.6))), 0.0);
    FragColor = vec4(objectColor * brightness, 1.0);
}
