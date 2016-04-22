
#version 330

in vec2 fTexCoords;

uniform sampler2D uTexture;

out vec4 outColor;


void main()
{
	outColor = vec4(texture(uTexture, fTexCoords).rgb, 1.0);
}
