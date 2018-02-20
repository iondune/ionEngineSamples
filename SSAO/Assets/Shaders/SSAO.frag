
#version 330

out vec4 outColor;
in vec2 fTexCoords;

uniform mat4 uProjectionMatrix;

uniform sampler2D tSceneNormals;
uniform sampler2D tSceneDepth;
uniform sampler2D texNoise;

uniform float radius;

const int kernelSize = 64;
const int noiseTexSize = 4;
uniform vec3 samples[kernelSize];


vec3 reconstructViewspacePosition(vec2 texCoords)
{
	float depth = texture(tSceneDepth, texCoords).r;
	vec3 ndc = vec3(texCoords, depth) * 2.0 - vec3(1.0);
	vec4 view = inverse(uProjectionMatrix) * vec4(ndc, 1.0);
	return view.xyz / view.w;
}

void main()
{
	// Reconstruct viewspace fragment position
	vec3 fragmentPosition = reconstructViewspacePosition(fTexCoords);

	// Read viewspace fragment normal from gbuffer
	vec3 normal = texture(tSceneNormals, fTexCoords).rgb * 2.0 - vec3(1.0);

	// Early exit - uninitialized normals (i.e. cleared depth)
	if (normal == vec3(-1.0))
	{
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	// Random vector (to orient hemisphere)
	vec2 noiseScale = vec2(textureSize(tSceneDepth, 0)) / float(noiseTexSize);
	vec3 randomVec = texture(texNoise, fTexCoords * noiseScale).xyz;

	// Tangent to view transform
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// Run samples
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// Sample position in view space
		vec3 sample = fragmentPosition + (TBN * samples[i]) * radius;

		// Transform sample to NDC (get texture coordinates)
		vec4 offset = vec4(sample, 1.0);
		offset = uProjectionMatrix * offset; // view to clip-space
		offset.xy /= offset.w; // clip to ndc
		offset.xy = offset.xy * 0.5 + 0.5; // transform to range 0.0 - 1.0

		if (offset.x > 1.0 || offset.y > 1.0 || offset.x < 0.0 || offset.y < 0.0)
		{
			continue; // skip samples outside of framebuffer
		}

		vec3 samplePosition = reconstructViewspacePosition(offset.xy);

		if (samplePosition.z >= sample.z) // check for crevice
		{
			if (abs(fragmentPosition.z - samplePosition.z) < radius) // check within radius
			{
				float rangeFactor = smoothstep(0.0, 1.0, radius / abs(fragmentPosition.z - samplePosition.z));
				occlusion += rangeFactor;
			}
		}
	}
	occlusion = 1.0 - (occlusion / kernelSize);

	vec3 Color = vec3(0.0);
	Color = vec3(occlusion);
	// Color = texture(texNoise, fTexCoords * noiseScale).xyz;
	// Color = normal * 0.5 + vec3(0.5);

	outColor = vec4(Color, 1.0);
}
