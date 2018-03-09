
#version 330

in vec2 fTexCoords;
out vec4 outColor;

uniform sampler2D tSceneColor;


void main()
{
	outColor.rgb = texture(tSceneColor, fTexCoords).rgb;
	outColor.a = 1.0;
}
