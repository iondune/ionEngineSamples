
#version 330

#define LIGHT_MAX 4

struct SDirectionalLight
{
	vec3 Color;
	vec3 Direction;
};

in vec3 fColor;
in vec3 fNormal;
in vec3 fBarycentric;
in float fSelected;

uniform int uDirectionalLightsCount;
uniform SDirectionalLight uDirectionalLights[LIGHT_MAX];
uniform vec3 uOutlineColor;

uniform float uSelectionOpacity;
uniform vec3 uSelectionColor;

out vec4 outColor;

float edgeFactor()
{
	vec3 d = fwidth(fBarycentric);
	vec3 a3 = smoothstep(vec3(0.0), d*1.5, fBarycentric);
	return min(min(a3.x, a3.y), a3.z);
}

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

	vec3 AmbientColor = vec3(0.3) * fColor;
	vec3 DiffuseColor = vec3(0.7) * fColor;

	vec3 Color = Diffuse * DiffuseColor + AmbientColor;
	vec3 Outline = uOutlineColor;

	if (fSelected > 0.5)
	{
		Color = mix(Color, uSelectionColor, uSelectionOpacity);
		Outline = mix(uSelectionColor, vec3(1.0), 0.5);
	}

	// outColor = vec4(Color, 1.0);
	outColor = vec4(mix(Outline, Color, edgeFactor()), 1.0);
	// outColor = vec4(vec3(edgeFactor()), 1.0);
	// outColor = vec4(nNormal * 0.5 + vec3(0.5), 1.0);
}
