uniform sampler2D texture;
uniform float flash_amount;

void main()
{
    vec4 color = texture2D(texture, gl_TexCoord[0].xy);
    
    color.r = color.r * (1 - flash_amount) + flash_amount;
    color.g = color.g * (1 - flash_amount) + flash_amount;
    color.b = color.b * (1 - flash_amount) + flash_amount;

    gl_FragColor = color * gl_Color;
}