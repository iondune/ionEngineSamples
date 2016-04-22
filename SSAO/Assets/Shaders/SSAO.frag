
#version 330

out vec4 FragColor;
in vec2 fTexCoords;

uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
uniform float radius;

// tile noise texture over screen based on screen dimensions divided by noise size
const float noiseTexSize = 128.0f;
const vec2 noiseScale = vec2(1600.0f/noiseTexSize, 900.0f/noiseTexSize);

uniform mat4 uProjectionMatrix;

void main()
{
    // Get input for SSAO algorithm
    vec3 fragPos = texture(gPositionDepth, fTexCoords).xyz;
    vec3 normal = texture(gNormal, fTexCoords).rgb;
    vec3 randomVec = texture(texNoise, fTexCoords * noiseScale).xyz;
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-space
        sample = fragPos + sample * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = uProjectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = -texture(gPositionDepth, offset.xy).w; // Get depth value of kernel sample

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);

    FragColor = vec4(vec3(occlusion), 1.0);
    // FragColor = vec4(texture(texNoise, fTexCoords * noiseScale).xyz, 1.0);
}