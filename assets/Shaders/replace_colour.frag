#version 120

uniform sampler2D texture;

uniform vec4 toReplace;
uniform vec4 replaceColour;

void main()
{
    vec4 color = texture2D(texture, gl_TexCoord[0].xy);

    if (color == toReplace)
    {
        color = replaceColour;
    }

    gl_FragColor = color * gl_Color;
}