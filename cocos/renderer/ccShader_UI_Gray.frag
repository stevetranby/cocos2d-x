const char* ccPositionTexture_GrayScale_frag = STRINGIFY(

\n#ifdef GL_ES\n
precision mediump float;
\n#endif

\nvarying vec4 v_fragmentColor;
\nvarying vec2 v_texCoord;
uniform float CC_alpha_value;

void main(void)
{
	vec4 c = texture2D(CC_Texture0, v_texCoord);
    if(c.a < 0.5)
        discard;
	gl_FragColor.xyz = vec3(0.2126*c.r + 0.7152*c.g + 0.0722*c.b);
    gl_FragColor.a = 0.5;
}
);
