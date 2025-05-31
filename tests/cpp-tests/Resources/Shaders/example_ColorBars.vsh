// http://www.cocos2d-iphone.org

attribute vec4 a_position;
attribute vec2 a_texCoord0;

#ifdef GL_ES
varying mediump vec2 v_texCoord0;
#else
varying vec2 v_texCoord0;
#endif

void main()
{
    gl_Position = CC_PMatrix * a_position;
	v_texCoord0 = a_texCoord0;
}

