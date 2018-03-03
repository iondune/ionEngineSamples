
#version 330

#define LIGHT_MAX 4

struct SDirectionalLight
{
	vec3 Color;
	vec3 Direction;
};

in vec3 fNormal;
in vec3 fDebugColor;
in vec2 fTexCoords;

uniform sampler2D uTexture;

uniform int uDirectionalLightsCount;
uniform SDirectionalLight uDirectionalLights[LIGHT_MAX];

uniform bool uDebugShowWeightsByJoint;
uniform bool uDebugShowWeightsByVertex;

out vec4 outColor;


void main()
{
	vec3 Diffuse = vec3(0);

	vec3 nNormal = normalize(fNormal);
	for (int i = 0; i < LIGHT_MAX && i < uDirectionalLightsCount; ++ i)
	{
		vec3 nLight = -normalize(uDirectionalLights[i].Direction);
		float Shading = clamp(dot(nNormal, nLight), 0.0, 1.0);

		Diffuse += Shading * uDirectionalLights[i].Color;
	}

	const vec3 AmbientColor = vec3(0.2);
	const vec3 DiffuseColor = vec3(0.8);
	vec3 TextureColor = texture(uTexture, fTexCoords).rgb;
	outColor = vec4((Diffuse * DiffuseColor + AmbientColor) * TextureColor, 1.0);
	// outColor = vec4(nNormal * 0.5 + vec3(0.5), 1.0);

	if (uDebugShowWeightsByJoint || uDebugShowWeightsByVertex)
	{
		outColor = vec4(fDebugColor, 0);
	}
}
