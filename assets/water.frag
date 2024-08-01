uniform sampler2D texture;
uniform sampler2D noise;

uniform vec2 textureStart;
uniform vec2 textureEnd;
uniform float time;
uniform vec2 worldOffset;

void main()
{
    float waveFrequency = 3;
    float waveHeight = 0.1;

    float noiseSpeed = 0.1;
    float noiseDampen = 0.2;

    float noiseSampleDivide = 8;

    vec2 noiseSampleCoords;
    noiseSampleCoords.x = mod(gl_TexCoord[0].x / noiseSampleDivide + worldOffset.x + time * noiseSpeed, 1.0);
    noiseSampleCoords.y = mod(gl_TexCoord[0].y / noiseSampleDivide + worldOffset.y + time * noiseSpeed, 1.0);

    vec4 noiseSample = texture2D(noise, noiseSampleCoords);
    float noiseCoordOffset = noiseSample.r * noiseDampen;

    vec2 texCoord;
    texCoord.x = mod(gl_TexCoord[0].x + worldOffset.x + noiseCoordOffset, textureEnd.x - textureStart.x) + textureStart.x;
    texCoord.y = mod(gl_TexCoord[0].y + worldOffset.y + sin(time + (gl_TexCoord[0].x) * waveFrequency + noiseCoordOffset) * waveHeight + noiseCoordOffset, textureEnd.y - textureStart.y) + textureEnd.y;

    vec4 color = texture2D(texture, texCoord);

    gl_FragColor = color * gl_Color;
}