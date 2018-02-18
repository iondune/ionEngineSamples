
#version 330

out vec4 outColor;
in vec2 fTexCoords;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

const int kernelSize = 64;
uniform float radius;


uniform mat4 uProjectionMatrix;

void main()
{
	vec3 Color = vec3(0.0);

	// Read from gbuffer
	vec3 fragPos = texture(gPositionDepth, fTexCoords).xyz;
	vec3 normal = texture(gNormal, fTexCoords).rgb;

	// Random vector (to orient hemisphere)
	const float noiseTexSize = 4.0f;
	vec2 noiseScale = vec2(textureSize(gPositionDepth, 0)) / noiseTexSize;
	vec3 randomVec = texture(texNoise, fTexCoords * noiseScale).xyz;

	// Tangent to view transform
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// Run samples
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// Sample position in in view space
		vec3 sample = fragPos + (TBN * samples[i]) * radius;

		// Transform sample to NDC
		vec4 offset = vec4(sample, 1.0);
		offset = uProjectionMatrix * offset; // view to clip-space
		offset.xy /= offset.w; // clip to ndc
		offset.xy = offset.xy * 0.5 + 0.5; // transform to range 0.0 - 1.0

		if (offset.x > 1.0 || offset.y > 1.0 || offset.x < 0.0 || offset.y < 0.0)
		{
			continue; // skip samples outside of framebuffer
		}

		// get sample depth
		float sampleDepth = -texture(gPositionDepth, offset.xy).w; // Get depth value of kernel sample

		if (sampleDepth == 0.0)
		{
			continue; // because we have no skybox, skip the fragments from depth clear
		}

		if (sampleDepth >= sample.z) // check for crevice
		{
			if (abs(fragPos.z - sampleDepth) < radius)
			{
				occlusion += smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
			}
		}
	}
	occlusion = 1.0 - (occlusion / kernelSize);

	Color = vec3(occlusion);
	// Color = texture(texNoise, fTexCoords * noiseScale).xyz;
	// Color = normal * 0.5 + vec3(0.5);

	outColor = vec4(Color, 1.0);
}
