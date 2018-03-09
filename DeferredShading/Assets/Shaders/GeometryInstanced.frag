
#version 330

in vec3 fColor;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;


void main()
{
	outColor = fColor;
	outNormal = vec3(0.5);
}
