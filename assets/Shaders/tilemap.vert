#version 330 core

uniform vec2 position;
uniform vec2 scale;

out vec4 fragColor;
out vec2 fragUV;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTextureUV;

void main()
{
    // gl_Position = vec4((1.0 + inPosition.x) * scale.x - 1.0 + position.x, (1.0 + inPosition.y) * scale.y -1.0 + position.y, 0.0, 1.0);
    gl_Position = vec4(inPosition.x * scale.x, inPosition.y * scale.y, 0.0, 1.0);
    fragColor = inColor;
    fragUV = inTextureUV;
}