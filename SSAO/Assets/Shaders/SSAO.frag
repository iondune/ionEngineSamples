
#version 330

in vec2 fTexCoords;

uniform sampler2D uColor;

out vec4 outColor;


void main()
{
	outColor = texture(uColor, fTexCoords);
}
