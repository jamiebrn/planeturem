#version 330 core

uniform sampler2D textureSampler;

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    // Circle edge cutout
    float centreRadius = sqrt(pow(fragUV.x - 0.5, 2) + pow(fragUV.y - 0.5, 2));

    if (centreRadius > 0.5)
    {
        FragColor = vec4(0, 0, 0, 0);
        return;
    }

    vec4 textureColor = texture(textureSampler, fragUV);
    FragColor = fragColor * textureColor;
}