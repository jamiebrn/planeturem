#version 330 core

uniform sampler2D texture;

// info about texture
uniform vec2 spriteSheetSize;
uniform vec4 textureRect;

// between 0 and 1
uniform float progress;

uniform int cropOption; // 0 for no pixel, 1 for greyscale

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    float normalizedX = (fragUV.x - textureRect.x / spriteSheetSize.x) / (textureRect.z / spriteSheetSize.x);
    float normalizedY = (fragUV.y - textureRect.y / spriteSheetSize.y) / (textureRect.w / spriteSheetSize.y);

    vec4 texColor = texture2D(texture, fragUV.xy);

    if ((atan(normalizedX - 0.5, normalizedY - 0.5) + 3.14159265) / (2 * 3.14159265) >= 1.0 - progress)
    {
        // Crop
        if (cropOption == 0)
        {
            // No color
            FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        }
        else if (cropOption == 1)
        {
            // Greyscale
            float greyscale = (texColor.r + texColor.g + texColor.b) / 3.0;
            FragColor = vec4(greyscale, greyscale, greyscale, texColor.a);
        }
    }
    else
    {
        // Default colour
        FragColor = texColor * fragColor;
    }
}