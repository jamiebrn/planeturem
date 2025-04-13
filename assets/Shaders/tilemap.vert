#version 330 core

uniform vec2 v_targetHalfSize;
uniform vec2 v_textureSize;

uniform vec2 position;
uniform vec2 scale;

out vec4 fragColor;
out vec2 fragUV;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTextureUV;

void main()
{
    vec2 ndcPosition = vec2((inPosition.x - v_targetHalfSize.x) / v_targetHalfSize.x, -(inPosition.y - v_targetHalfSize.y) / v_targetHalfSize.y);
    gl_Position = vec4((ndcPosition.x + 1.0) * scale.x + position.x, (ndcPosition.y - 1.0) * scale.y + position.y, 0.0, 1.0);

    fragColor = inColor / 255.0;

    if (v_textureSize == vec2(0, 0))
    {
        fragUV = inTextureUV;
    }
    else
    {
        fragUV = vec2(inTextureUV.x / v_textureSize.x, inTextureUV.y / v_textureSize.y);
    }
}