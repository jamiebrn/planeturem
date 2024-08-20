uniform sampler2D texture;

uniform float waterHeight;

void main()
{
    int textureSize = 128; // must be updated if sprite sheet size changes

    // Cut texture to fit water height
    

    // Get colour one "pixel" in texture up
    vec4 colourUpwards = texture2D(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 1.0 / textureSize));

    vec4 colour = texture2D(texture, gl_TexCoord[0].xy);

    if (colour.a != 0)
    {
        gl_FragColor = colour * gl_Color;
    }
    else if(colourUpwards.a != 0)
    {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }


    // float waveFrequency = 2;
    // float waveHeight = 0.05;

    // float noiseSpeed = 0.05;
    // float noiseDampen = 0.6;

    // float noiseSampleDivide = 4;

    // vec2 noiseSampleCoords;
    // noiseSampleCoords.x = mod(gl_TexCoord[0].x / noiseSampleDivide + time * noiseSpeed, 1.0);
    // noiseSampleCoords.y = mod(gl_TexCoord[0].y / noiseSampleDivide + time * noiseSpeed, 1.0);

    // vec2 noiseTwoSampleCoords;
    // noiseTwoSampleCoords.x = mod(gl_TexCoord[0].x / noiseSampleDivide - time * noiseSpeed, 1.0);
    // noiseTwoSampleCoords.y = mod(gl_TexCoord[0].y / noiseSampleDivide - time * noiseSpeed, 1.0);

    // vec4 noiseSample = texture2D(noise, noiseSampleCoords);
    // vec4 noiseTwoSample = texture2D(noiseTwo, noiseTwoSampleCoords);

    // vec4 noiseMixed = mix(noiseSample, noiseTwoSample, 0.5);

    // float noiseCoordOffset = noiseMixed.r * noiseDampen;

    // vec2 texCoord;
    // texCoord.x = mod(gl_TexCoord[0].x + noiseCoordOffset, 1.0);
    // texCoord.y = mod(gl_TexCoord[0].y + sin(time + (gl_TexCoord[0].x + worldOffset.x * noiseSampleDivide) * waveFrequency) * waveHeight + noiseCoordOffset, 1.0);
    // // texCoord.y = gl_TexCoord[0].y + noiseCoordOffset;

    // vec4 texColor = texture2D(texture, texCoord);
    
    // vec4 color;
    // if (texColor.a == 0.0)
    //     color = waterColor;
    // else
    //     color = mix(texColor, waterColor, 0.7);
    
    // // color = mix(noiseSample, noiseTwoSample, 0.5);
    // // color = noiseMixed;
    // // color = noiseSample;

    // gl_FragColor = color * gl_Color;
}