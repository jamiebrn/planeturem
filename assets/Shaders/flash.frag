#version 330 core

uniform sampler2D texture;
uniform float flash_amount;

out vec4 FragColor;

in vec4 fragColor;
in vec2 fragUV;

void main()
{
    vec4 color = texture2D(texture, fragUV.xy);
    
    color.r = color.r * (1 - flash_amount) + flash_amount;
    color.g = color.g * (1 - flash_amount) + flash_amount;
    color.b = color.b * (1 - flash_amount) + flash_amount;

    FragColor = color * color;
}