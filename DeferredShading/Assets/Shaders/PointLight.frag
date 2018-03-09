
#version 330

in vec3 fColor;
in vec3 fPosition;

out vec4 outColor;

uniform mat4 uInvProjectionMatrix;
uniform mat4 uInvViewMatrix;

uniform sampler2D tSceneColor;
uniform sampler2D tSceneNormals;
uniform sampler2D tSceneDepth;


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
	vec2 TexCoords = gl_FragCoord.xy / vec2(textureSize(tSceneDepth, 0));

	outColor = vec4(0.0, 0.0, 0.0, 1.0);

	vec3 normal = texture(tSceneNormals, TexCoords).rgb;
	normal = normal * 2.0 - vec3(1.0);

	vec3 world = reconstructWorldspacePosition(TexCoords);
	vec3 light = normalize(fPosition - world);
	float distance = length(fPosition - world);

	float diffuse = 0.6 * clamp(dot(normal, light), 0.0, 1.0);


	const float radius = 10.0;
	float attenuation = 1.0 - (distance / radius);

	outColor.rgb = texture(tSceneColor, TexCoords).rgb * diffuse * fColor * attenuation;
	// outColor.rgb = fColor;

	if (abs(length(normal)) < 0.01)
	{
		outColor.rgb = texture(tSceneColor, TexCoords).rgb;
		outColor.a = 0.0;
	}
}
