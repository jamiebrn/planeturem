#version 120

uniform sampler2D texture;
uniform sampler2D noise;
uniform sampler2D noiseTwo;

uniform vec2 spriteSheetSize;
uniform vec4 textureRect;

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
    
    vec4 color = texColor;
    if (texColor.a == 0.0) color = waterColor;
    else if (waterColor != vec4(1.0, 1.0, 1.0, 1.0)) color = mix(texColor, waterColor, 0.7);

    gl_FragColor = color * gl_Color;
}