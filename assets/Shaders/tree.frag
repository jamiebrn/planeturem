uniform sampler2D texture;
uniform float time;

void main()
{
    vec4 color;

    if (gl_TexCoord[0].y < 0.5)
        color = texture2D(texture, vec2(gl_TexCoord[0].x + time, gl_TexCoord[0].y));
    else
        color = texture2D(texture, gl_TexCoord[0].xy);

    gl_FragColor = color;
}