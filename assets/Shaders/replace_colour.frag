#version 330 core

uniform sampler2D texture;

uniform vec4 replaceKeys[10];
uniform vec4 replaceValues[10];
uniform int replaceKeyCount;

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    vec4 color = texture2D(texture, fragUV.xy);

    for (int i = 0; i < replaceKeyCount; i++)
    {
        if (color == replaceKeys[i])
        {
            color = replaceValues[i];
            break;
        }
    }

    FragColor = color * fragColor;
}