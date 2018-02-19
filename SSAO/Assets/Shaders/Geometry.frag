
#version 330

#define LIGHT_MAX 4

struct SMaterial
{
	vec3 AmbientColor;
	vec3 DiffuseColor;
};

in vec3 fNormal;
in vec3 fViewPosition;

uniform SMaterial uMaterial;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;

void main()
{
	outColor = uMaterial.DiffuseColor;
	outNormal = normalize(fNormal) * 0.5 + vec3(0.5);

	// Per-face normals
	vec3 fdx = dFdx(fViewPosition);
	vec3 fdy = dFdy(fViewPosition);
	// outNormal = normalize(cross(fdx, fdy));
}
