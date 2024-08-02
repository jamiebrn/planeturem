uniform sampler2D texture;
uniform sampler2D noise;

uniform vec4 waterColor;

uniform float time;
uniform vec2 worldOffset;

void main()
{
    float waveFrequency = 2;
    float waveHeight = 0.05;

    float noiseSpeed = 0.1;
    float noiseDampen = 0.3;

    float noiseSampleDivide = 4;

    vec2 noiseSampleCoords;
    noiseSampleCoords.x = mod((gl_TexCoord[0].x + worldOffset.x) / noiseSampleDivide + worldOffset.x + time * noiseSpeed, 1.0);
    noiseSampleCoords.y = mod(gl_TexCoord[0].y / noiseSampleDivide + worldOffset.y + time * noiseSpeed, 1.0);

    vec4 noiseSample = texture2D(noise, noiseSampleCoords);
    float noiseCoordOffset = noiseSample.r * noiseDampen;

    vec2 texCoord;
    texCoord.x = gl_TexCoord[0].x + worldOffset.x + noiseCoordOffset;
    texCoord.y = gl_TexCoord[0].y + sin(time + (gl_TexCoord[0].x + worldOffset.x) * waveFrequency) * waveHeight + noiseCoordOffset;

    vec4 texColor = texture2D(texture, texCoord);
    
    vec4 color;
    if (texColor.a == 0.0)
        color = waterColor;
    else
        color = mix(texColor, waterColor, 0.7);

    gl_FragColor = color * gl_Color;
}