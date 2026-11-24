#version 330 core
in vec3 worldNormal;
out vec4 FragColor;
// RGB는 C++의 obstacleColor와 dogColor 등에서 objectColor로 전달된다.
uniform vec3 objectColor;

void main()
{
    // 0.65는 기본 밝기이고 0.35는 면 방향에 따른 밝기 차이다.
    float brightness = 0.65 + 0.35 * max(dot(normalize(worldNormal), normalize(vec3(0.4, 1.0, 0.6))), 0.0);
    FragColor = vec4(objectColor * brightness, 1.0);
}
