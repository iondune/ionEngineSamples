#version 150

in vec2 fTexCoords;

uniform sampler2D uTexture;

out vec4 outColor;

void main()
{
	outColor = texture(uTexture, fTexCoords);
	if (outColor.a < 0.5)
		discard;
}
