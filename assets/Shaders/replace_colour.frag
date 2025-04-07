#version 330 core

uniform sampler2D texture;

uniform vec4 replaceKeys[10];
uniform vec4 replaceValues[10];
uniform int replaceKeyCount;

void main()
{
    vec4 color = texture2D(texture, gl_TexCoord[0].xy);

    for (int i = 0; i < replaceKeyCount; i++)
    {
        if (color == replaceKeys[i])
        {
            color = replaceValues[i];
            break;
        }
    }

    gl_FragColor = color * gl_Color;
}