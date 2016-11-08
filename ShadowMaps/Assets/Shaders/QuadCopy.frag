
#version 330

in vec2 fTexCoords;

uniform sampler2D uTexture;

out vec4 outColor;


void main()
{
	// outColor = vec4(vec3(pow(texture(uTexture, fTexCoords).r / 20.0, 10.0)), 1.0);
	outColor = vec4(vec3(texture(uTexture, fTexCoords).r), 1.0);
}
