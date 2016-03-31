#version 150

in vec2 fTexCoord;
in vec2 fPosition;

uniform sampler2D uTexture;

out vec4 outColor;


void main()
{
	outColor = vec4(texture(uTexture, fTexCoord).rgb * cos(length(fPosition) * 1.6), 1.0);
}
