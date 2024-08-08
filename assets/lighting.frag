uniform sampler2D texture;
uniform sampler2D lightingTexture;

uniform vec4 unlitColor;
uniform float darkness;

void main()
{
    vec4 textureColor = texture2D(texture, gl_TexCoord[0].xy);
    vec4 lightSample = texture2D(lightingTexture, gl_TexCoord[0].xy);

    textureColor = mix(textureColor, vec4(0.01, 0.0, 0.03, 1.0), max(darkness - lightSample.a, 0.0f));

    gl_FragColor = textureColor * gl_Color;
}