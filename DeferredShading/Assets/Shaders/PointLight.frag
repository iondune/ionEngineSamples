
#version 330

out vec4 outColor;
in vec2 fTexCoords;

uniform mat4 uInvProjectionMatrix;
uniform mat4 uInvViewMatrix;

uniform sampler2D tSceneColor;
uniform sampler2D tSceneNormals;
uniform sampler2D tSceneDepth;

uniform vec3 uColor;
uniform vec3 uPosition;

vec3 reconstructWorldspacePosition(vec2 texCoords)
{
	float depth = texture(tSceneDepth, texCoords).r;
	vec3 ndc = vec3(texCoords, depth) * 2.0 - vec3(1.0);
	vec4 view = uInvProjectionMatrix * vec4(ndc, 1.0);
	view.xyz /= view.w;
	vec4 world = uInvViewMatrix * vec4(view.xyz, 1.0);
	return world.xyz;
}

void main()
{
	outColor = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 normal = texture(tSceneNormals, fTexCoords).rgb;
	normal = normal * 2.0 - vec3(1.0);

	vec3 world = reconstructWorldspacePosition(fTexCoords);
	vec3 light = normalize(uPosition - world);
	float distance = length(uPosition - world);

	float diffuse = 0.6 * clamp(dot(normal, light), 0.0, 1.0);


	const float cutoff = 0.005;
	const float radius = 3.0;

	float denom = distance/radius + 1;
	float attenuation = max((1 / (denom*denom) - cutoff) / (1 - cutoff), 0);

	outColor.rgb = texture(tSceneColor, fTexCoords).rgb * diffuse * uColor * attenuation;
}
