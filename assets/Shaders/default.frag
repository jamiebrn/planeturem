#version 330 core

uniform sampler2D textureSampler;

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    vec4 textureColor = texture(textureSampler, fragUV);
    FragColor = fragColor * textureColor;
}