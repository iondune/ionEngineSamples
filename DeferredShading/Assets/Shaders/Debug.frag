
#version 330

out vec4 outColor;
in vec2 fTexCoords;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;

uniform sampler2D tSceneColor;
uniform sampler2D tSceneNormals;
uniform sampler2D tSceneDepth;

uniform int uMode;

/* Gives us a linear mapping of depth values from 0.0 to 1.0 */
/* Useful for debug visualization of depths - otherwise it's hard */
/* to see anything not close to the near plane */
float linearizeDepth(in float depth)
{
	float zNear = 0.1;
	float zFar  = 100.0;
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

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
	outColor = vec4(vec3(0.5), 1.0);

	if (uMode == 0)
	{
		vec3 normal = texture(tSceneNormals, fTexCoords).rgb;
		normal = normal * 2.0 - vec3(1.0);

		const vec3 light = normalize(vec3(3.0, 4.0, 5.0));
		const float ambient = 0.4;
		float diffuse = 0.6 * clamp(dot(normal, light), 0.0, 1.0);

		outColor.rgb = texture(tSceneColor, fTexCoords).rgb * (ambient + diffuse);
	}
	if (uMode == 1) // Color pass-thru
	{
		outColor.rgb = texture(tSceneColor, fTexCoords).rgb;
	}
	else if (uMode == 2) // Debug Normals
	{
		outColor.rgb = texture(tSceneNormals, fTexCoords).rgb;
	}
	else if (uMode == 3) // Debug Depth
	{
		float depth = texture(tSceneDepth, fTexCoords).r;
		float linDepth = linearizeDepth(depth);

		outColor.r = depth;
		outColor.g = linDepth;
		outColor.b = sin(linDepth * 3.1415 * 2.0);
	}
	else if (uMode == 4) // Debug Position
	{
		vec3 world = reconstructWorldspacePosition(fTexCoords);

		outColor.rgb = mod(world + vec3(0.0, 0.5, 0.0), vec3(1.0));
	}
}
