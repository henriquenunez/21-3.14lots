#version 330 core
//Fragment Shader

uniform vec3 color;
out vec4 FragColor;

void main()
{
    //FragColor = vec4(1.0f);
    FragColor = vec4(color, 1.0f);
}
