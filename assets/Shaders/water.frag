#version 120

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

    // Mix with surrounding water
    // vec4 adjustedWaterColor = mix(waterColor, surroundingWaterColors[0], max(0.5 - (normalizedTexY / noiseSampleDivide), 0.0)); // mix with up
    // adjustedWaterColor = mix(adjustedWaterColor, surroundingWaterColors[1], max((normalizedTexX / noiseSampleDivide) - 0.5, 0.0)); // mix with right
    // adjustedWaterColor = mix(adjustedWaterColor, surroundingWaterColors[2], max((normalizedTexY / noiseSampleDivide) - 0.5, 0.0)); // mix with down
    // adjustedWaterColor = mix(adjustedWaterColor, surroundingWaterColors[3], max(0.5 - (normalizedTexX / noiseSampleDivide), 0.0)); // mix with left

    // adjustedWaterColor = mix(adjustedWaterColor, surroundingWaterColors[4], max((sqrt(0.5) - sqrt(pow(max(normalizedTexX / noiseSampleDivide, 0.5), 2) + pow(max(normalizedTexY / noiseSampleDivide, 0.5), 2))) / sqrt(0.5) * 0.5, 0.0)); // mix with up-left
    // adjustedWaterColor = mix(adjustedWaterColor, surroundingWaterColors[5], max((sqrt(0.5) - sqrt(pow(max(1.0 - normalizedTexX / noiseSampleDivide, 0.5), 2) + pow(max(normalizedTexY / noiseSampleDivide, 0.5), 2))) / sqrt(0.5) * 0.5, 0.0)); // mix with up-right
    // adjustedWaterColor = mix(adjustedWaterColor, surroundingWaterColors[6], max((sqrt(0.5) - sqrt(pow(max(1.0 - normalizedTexX / noiseSampleDivide, 0.5), 2) + pow(max(1.0 - normalizedTexY / noiseSampleDivide, 0.5), 2))) / sqrt(0.5) * 0.5, 0.0)); // mix with down-right
    // adjustedWaterColor = mix(adjustedWaterColor, surroundingWaterColors[7], max((sqrt(0.5) - sqrt(pow(max(normalizedTexX / noiseSampleDivide, 0.5), 2) + pow(max(1.0 - normalizedTexY / noiseSampleDivide, 0.5), 2))) / sqrt(0.5) * 0.5, 0.0)); // mix with down-left

    // vec4 blendedColor = waterColor;

    float weightCenter = (1.0 - abs(normalizedTexX / noiseSampleDivide - 0.5)) * (1.0 - abs(normalizedTexY / noiseSampleDivide - 0.5));

    float weightUp = max(0.0, 1.0 - normalizedTexY / noiseSampleDivide);
    float weightDown = max(0.0, normalizedTexY / noiseSampleDivide);
    float weightLeft = max(0.0, 1.0 - normalizedTexX / noiseSampleDivide);
    float weightRight = max(0.0, normalizedTexX / noiseSampleDivide);

    float weightUpLeft = weightUp * weightLeft * 1.0 / sqrt(2);
    float weightUpRight = weightUp * weightRight * 1.0 / sqrt(2);
    float weightDownLeft = weightDown * weightLeft * 1.0 / sqrt(2);
    float weightDownRight = weightDown * weightRight * 1.0 / sqrt(2);

    // Normalize weights
    float totalWeight = weightCenter + weightUp + weightDown + weightLeft + weightRight;
                    //   + weightUpLeft + weightUpRight + weightDownLeft + weightDownRight;

    // Blend colors based on weights
    vec4 blendedColor =
          waterColor * (weightCenter / totalWeight)
        + surroundingWaterColors[0] * (weightUp / totalWeight)
        + surroundingWaterColors[1] * (weightRight / totalWeight)
        + surroundingWaterColors[2] * (weightDown / totalWeight)
        + surroundingWaterColors[3] * (weightLeft / totalWeight);
        // + surroundingWaterColors[4] * (weightUpLeft / totalWeight)
        // + surroundingWaterColors[5] * (weightUpRight / totalWeight)
        // + surroundingWaterColors[6] * (weightDownRight / totalWeight)
        // + surroundingWaterColors[7] * (weightDownLeft / totalWeight);

    vec4 color = texColor;
    if (texColor.a == 0.0) color = blendedColor;
    else if (waterColor != vec4(1.0, 1.0, 1.0, 1.0)) color = mix(texColor, blendedColor, 0.7);

    gl_FragColor = color * gl_Color;
}