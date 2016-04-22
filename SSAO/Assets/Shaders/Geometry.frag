
#version 330

#define LIGHT_MAX 4

struct SMaterial
{
	vec3 AmbientColor;
	vec3 DiffuseColor;
};

in vec3 fNormal;
in vec3 fPosition;

uniform SMaterial uMaterial;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec3 outNormal;

const float NEAR = 0.1; // projection matrix's near plane
const float FAR = 50.0; // projection matrix's far plane
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // Back to NDC
	return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}

void main()
{
	outColor = uMaterial.DiffuseColor;
	outPosition.xyz = fPosition;
	outPosition.a = LinearizeDepth(gl_FragCoord.z);
	outNormal = normalize(fNormal);
}
