#version 330 core

uniform sampler2D lightingTexture;

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    float lightStrength = texture(lightingTexture, fragUV).r;
    vec4 textureColor = vec4(lightStrength, lightStrength, lightStrength, lightStrength);
    FragColor = fragColor * textureColor;
}