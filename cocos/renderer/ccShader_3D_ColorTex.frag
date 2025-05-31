
const char* cc3D_ColorTex_frag = R"(

#ifdef GL_ES
varying highp vec2 TextureCoordOut;
uniform lowp vec4 u_color;
#else
varying vec2 TextureCoordOut;
uniform vec4 u_color;
#endif

void main(void)
{
    gl_FragColor = texture2D(CC_Texture0, TextureCoordOut) * u_color;
}
)";
