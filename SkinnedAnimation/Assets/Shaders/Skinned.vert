
#version 330

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoords;
in vec4 vBoneWeights;
in vec4 vBoneIndices;

#define JOINT_MAX 32
uniform mat4 uSkinningMatrix[JOINT_MAX];

uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

uniform bool uDebugDoSkin;
uniform bool uDebugShowWeightsByJoint;
uniform bool uDebugShowWeightsByVertex;
uniform int uDebugWeightSelector;

out vec3 fNormal;
out vec3 fDebugColor;
out vec2 fTexCoords;


mat4 MakeSkinningMatrix(vec4 boneWeights0, ivec4 boneIndices0)
{
	mat4 result = boneWeights0.x * uSkinningMatrix[boneIndices0.x];
	result = result + boneWeights0.y * uSkinningMatrix[boneIndices0.y];
	result = result + boneWeights0.z * uSkinningMatrix[boneIndices0.z];
	result = result + boneWeights0.w * uSkinningMatrix[boneIndices0.w];
	return result;
}

void main()
{
	mat4 SkinningMatrix = MakeSkinningMatrix(vBoneWeights, ivec4(vBoneIndices));

	if (! uDebugDoSkin)
	{
		SkinningMatrix = mat4(1);
	}

	fDebugColor = vec3(0.0);

	if (uDebugShowWeightsByJoint)
	{
		if (int(vBoneIndices.x) == uDebugWeightSelector)
		{
			fDebugColor.gb = vec2(vBoneWeights.x);
		}
		else if (int(vBoneIndices.y) == uDebugWeightSelector)
		{
			fDebugColor.gb = vec2(vBoneWeights.y);
		}
		else if (int(vBoneIndices.z) == uDebugWeightSelector)
		{
			fDebugColor.gb = vec2(vBoneWeights.z);
		}
		else if (int(vBoneIndices.w) == uDebugWeightSelector)
		{
			fDebugColor.gb = vec2(vBoneWeights.w);
		}
	}

	if (uDebugShowWeightsByVertex)
	{
		fDebugColor.rgb = vBoneWeights.rgb;
	}

	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * SkinningMatrix * vec4(vPosition, 1.0);
	fNormal = (uNormalMatrix * inverse(transpose(SkinningMatrix)) * vec4(vNormal, 0.0)).xyz;
	fTexCoords = vTexCoords;
}
