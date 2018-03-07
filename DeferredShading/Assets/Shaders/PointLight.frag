
#version 330

out vec4 outColor;
in vec2 fTexCoords;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;

uniform sampler2D tSceneColor;
uniform sampler2D tSceneNormals;
uniform sampler2D tSceneDepth;

uniform vec3 uColor;
uniform vec3 uPosition;

vec3 reconstructWorldspacePosition(vec2 texCoords)
{
	float depth = texture(tSceneDepth, texCoords).r;
	vec3 ndc = vec3(texCoords, depth) * 2.0 - vec3(1.0);
	vec4 view = inverse(uProjectionMatrix) * vec4(ndc, 1.0);
	view.xyz /= view.w;
	vec4 world = inverse(uViewMatrix) * vec4(view.xyz, 1.0);
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

	const float a = 1.0;
	const float b = 5.0;
	const float c = 5.0;

	float attenuation = 1.0 / (1.0 + b * distance + c * distance * distance);

	outColor.rgb = texture(tSceneColor, fTexCoords).rgb * diffuse * uColor;
}
