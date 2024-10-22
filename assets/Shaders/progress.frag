#version 120

uniform sampler2D texture;

// info about texture
uniform vec2 spriteSheetSize;
uniform vec4 textureRect;

// between 0 and 1
uniform float progress;

void main()
{
    float textureProgress = (gl_TexCoord[0].x - textureRect.x / spriteSheetSize.x) / (textureRect.z / spriteSheetSize.x);

    if (textureProgress > progress)
    {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }

    vec4 texColor = texture2D(texture, gl_TexCoord[0].xy);

    gl_FragColor = texColor * gl_Color;
}