uniform sampler2D texture;
uniform sampler2D noise;
uniform sampler2D noiseTwo;

uniform vec4 waterColor;

uniform float time;

void main()
{
    float waveFrequency = 1 * 3.14159265359;
    float waveHeight = 0.05;

    float noiseSpeed = 0.05;
    float noiseDampen = 0.6;

    float noiseSampleDivide = 4;

    vec2 noiseSampleCoords;
    noiseSampleCoords.x = mod(gl_TexCoord[0].x / noiseSampleDivide + time * noiseSpeed, 1.0);
    noiseSampleCoords.y = mod(gl_TexCoord[0].y / noiseSampleDivide + time * noiseSpeed, 1.0);

    vec2 noiseTwoSampleCoords;
    noiseTwoSampleCoords.x = mod(gl_TexCoord[0].x / noiseSampleDivide - time * noiseSpeed, 1.0);
    noiseTwoSampleCoords.y = mod(gl_TexCoord[0].y / noiseSampleDivide - time * noiseSpeed, 1.0);

    vec4 noiseSample = texture2D(noise, noiseSampleCoords);
    vec4 noiseTwoSample = texture2D(noiseTwo, noiseTwoSampleCoords);

    vec4 noiseMixed = mix(noiseSample, noiseTwoSample, 0.5);

    float noiseCoordOffset = noiseMixed.r * noiseDampen;

    vec2 texCoord;
    texCoord.x = mod(gl_TexCoord[0].x + noiseCoordOffset, 1.0);

    float wave = sin(waveFrequency * gl_TexCoord[0].x + time) * noiseCoordOffset * 0.15;
    texCoord.y = mod(gl_TexCoord[0].y + noiseCoordOffset + wave, 1.0);

    vec4 texColor = texture2D(texture, texCoord);
    
    vec4 color;
    if (texColor.a == 0.0) color = waterColor;
    else color = mix(texColor, waterColor, 0.7);

    gl_FragColor = color * gl_Color;
}