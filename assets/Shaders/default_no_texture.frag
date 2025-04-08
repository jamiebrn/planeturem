#version 330 core

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    FragColor = fragColor;
}