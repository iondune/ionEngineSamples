
#version 330

struct SMaterial
{
	vec3 AmbientColor;
	vec3 DiffuseColor;
};

in vec3 fNormal;

uniform SMaterial uMaterial;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;


void main()
{
	outColor = uMaterial.DiffuseColor;
	outNormal = normalize(fNormal) * 0.5 + vec3(0.5);
}
