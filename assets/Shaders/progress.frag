#version 330 core

uniform sampler2D texture;

// info about texture
uniform vec2 spriteSheetSize;
uniform vec4 textureRect;

// between 0 and 1
uniform float progress;

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    float textureProgress = (fragUV.x - textureRect.x / spriteSheetSize.x) / (textureRect.z / spriteSheetSize.x);

    if (textureProgress > progress)
    {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }

    vec4 texColor = texture2D(texture, fragUV.xy);

    FragColor = texColor * fragColor;
}