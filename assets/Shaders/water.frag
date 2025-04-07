#version 330 core

uniform sampler2D texture;
uniform sampler2D noise;
uniform sampler2D noiseTwo;

uniform vec2 spriteSheetSize;
uniform vec4 textureRect;

uniform vec4 surroundingWaterColors[8];
uniform vec4 waterColor;

uniform float time;

float mod_range(float value, float low, float high)
{
    return mod(value - low, high - low) + low;
}

void main()
{
    float waveFrequency = 1 * 3.14159265359;
    float waveHeight = 0.05;

    float noiseSpeed = 0.05;
    float noiseDampen = 0.6;

    float noiseSampleDivide = 4;

    float texU = textureRect.x / spriteSheetSize.x;
    float texV = textureRect.y / spriteSheetSize.y;
    float texWidth = textureRect.z / spriteSheetSize.x;
    float texHeight = textureRect.w / spriteSheetSize.y;

    float normalizedTexX = (gl_TexCoord[0].x - texU) / texWidth;
    float normalizedTexY = (gl_TexCoord[0].y - texV) / texHeight;

    vec2 noiseSampleCoords;
    noiseSampleCoords.x = mod(normalizedTexX / noiseSampleDivide + time * noiseSpeed, 1.0);
    noiseSampleCoords.y = mod(normalizedTexY / noiseSampleDivide + time * noiseSpeed, 1.0);

    vec2 noiseTwoSampleCoords;
    noiseTwoSampleCoords.x = mod(normalizedTexX / noiseSampleDivide - time * noiseSpeed, 1.0);
    noiseTwoSampleCoords.y = mod(normalizedTexY / noiseSampleDivide - time * noiseSpeed, 1.0);

    vec4 noiseSample = texture2D(noise, noiseSampleCoords);
    vec4 noiseTwoSample = texture2D(noiseTwo, noiseTwoSampleCoords);

    vec4 noiseMixed = mix(noiseSample, noiseTwoSample, 0.5);

    float noiseCoordOffset = noiseMixed.r * noiseDampen;

    vec2 texCoord;
    texCoord.x = mod_range(gl_TexCoord[0].x + noiseCoordOffset * texWidth, texU, texU + texWidth);

    float wave = sin(waveFrequency * normalizedTexX + time) * noiseCoordOffset * 0.15;
    texCoord.y = mod_range(gl_TexCoord[0].y + noiseCoordOffset * texHeight + wave * texHeight, texV, texV + texHeight);

    vec4 texColor = texture2D(texture, texCoord);

    const float edgeBlend = 0.3;

    float centreWeight = min((1.0 - abs(normalizedTexX / noiseSampleDivide - 0.5)), (1.0 - abs(normalizedTexY / noiseSampleDivide - 0.5)));
    float upWeight = max((edgeBlend - (normalizedTexY / noiseSampleDivide)) / edgeBlend * 0.5, 0.0);
    float rightWeight = max(((normalizedTexX / noiseSampleDivide) - (1.0 - edgeBlend)) / edgeBlend * 0.5, 0.0);
    float downWeight = max(((normalizedTexY / noiseSampleDivide) - (1.0 - edgeBlend)) / edgeBlend * 0.5, 0.0);
    float leftWeight = max((edgeBlend - (normalizedTexX / noiseSampleDivide)) / edgeBlend * 0.5, 0.0);
    float upLeftWeight = max((edgeBlend - sqrt(pow(max(normalizedTexX / noiseSampleDivide, 0.0), 2) + pow(max(normalizedTexY / noiseSampleDivide, 0.0), 2))) / edgeBlend * 0.5, 0.0);
    float upRightWeight = max((edgeBlend - sqrt(pow(max(1.0 - normalizedTexX / noiseSampleDivide, 0.0), 2) + pow(max(normalizedTexY / noiseSampleDivide, 0.0), 2))) / edgeBlend * 0.5, 0.0);
    float downRightWeight = max((edgeBlend - sqrt(pow(max(1.0 - normalizedTexX / noiseSampleDivide, 0.0), 2) + pow(max(1.0 - normalizedTexY / noiseSampleDivide, 0.0), 2))) / edgeBlend * 0.5, 0.0);
    float downLeftWeight = max((edgeBlend - sqrt(pow(max(normalizedTexX / noiseSampleDivide, 0.0), 2) + pow(max(1.0 - normalizedTexY / noiseSampleDivide, 0.0), 2))) / edgeBlend * 0.5, 0.0);

    float totalWeight = centreWeight + upWeight + rightWeight + downWeight + leftWeight + upLeftWeight + upRightWeight + downRightWeight + downLeftWeight;

    vec4 adjustedWaterColor = (waterColor * centreWeight + surroundingWaterColors[0] * upWeight + surroundingWaterColors[1] * rightWeight
        + surroundingWaterColors[2] * downWeight + surroundingWaterColors[3] * leftWeight + surroundingWaterColors[4] * upLeftWeight + surroundingWaterColors[5] * upRightWeight
        + surroundingWaterColors[6] * downRightWeight + surroundingWaterColors[7] * downLeftWeight) / totalWeight;
    
    vec4 color = texColor;
    if (texColor.a == 0.0) color = adjustedWaterColor;
    else if (waterColor != vec4(1.0, 1.0, 1.0, 1.0)) color = mix(texColor, adjustedWaterColor, 0.7);

    gl_FragColor = color * gl_Color;
}