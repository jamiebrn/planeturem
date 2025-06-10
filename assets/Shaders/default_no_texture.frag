#version 330 core

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    if (fragUV.x > 0)
    {
        FragColor = vec4(1, 1, 1, 1);
        return;
    }
    FragColor = fragColor;
}