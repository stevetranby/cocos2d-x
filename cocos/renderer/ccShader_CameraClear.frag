
const char* ccCameraClearFrag = R"(

#ifdef GL_ES
varying highp vec2 v_texCoord;
varying lowp vec3 v_color;
#else
varying vec2 v_texCoord;
varying vec4 v_color;
#endif
void main()
{
    gl_FragColor = v_color;
}
)";
