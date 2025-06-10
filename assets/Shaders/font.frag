#version 330 core

uniform sampler2D textureSampler;

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    vec4 textureColor = vec4(1.0, 1.0, 1.0, texture(textureSampler, fragUV).r);
    FragColor = fragColor * textureColor;
}