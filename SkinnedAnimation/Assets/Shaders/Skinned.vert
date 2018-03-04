
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
uniform bool uUseDualQuaternions;
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

mat4 MakeDualQuaternionMatrix(vec4 boneWeights0, ivec4 boneIndices0)
{
//
// The following license applies strictly to the contents of this function and
// not the remainder of this GLSL file.
//
// The MIT License (MIT)
//
// Copyright (c) Original Work Chinedu Francis Nwafili
// Copyright (c) Modified Work Ian Dunn
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

	vec4 weightedRotQuat =
		uSkinningMatrix[boneIndices0.x][0] * boneWeights0.x +
		uSkinningMatrix[boneIndices0.y][0] * boneWeights0.y +
		uSkinningMatrix[boneIndices0.z][0] * boneWeights0.z +
		uSkinningMatrix[boneIndices0.w][0] * boneWeights0.w;

	vec4 weightedTransQuat =
		uSkinningMatrix[boneIndices0.x][1] * boneWeights0.x +
		uSkinningMatrix[boneIndices0.y][1] * boneWeights0.y +
		uSkinningMatrix[boneIndices0.z][1] * boneWeights0.z +
		uSkinningMatrix[boneIndices0.w][1] * boneWeights0.w;

	float xRot = weightedRotQuat[0];
	float yRot = weightedRotQuat[1];
	float zRot = weightedRotQuat[2];
	float wRot = weightedRotQuat[3];

	float rotQuatMagnitude = sqrt(xRot * xRot + yRot * yRot + zRot * zRot + wRot * wRot);
	weightedRotQuat = weightedRotQuat / rotQuatMagnitude;
	weightedTransQuat = weightedTransQuat / rotQuatMagnitude;

	float xR = weightedRotQuat[0];
	float yR = weightedRotQuat[1];
	float zR = weightedRotQuat[2];
	float wR = weightedRotQuat[3];
	float xT = weightedTransQuat[0];
	float yT = weightedTransQuat[1];
	float zT = weightedTransQuat[2];
	float wT = weightedTransQuat[3];
	float t0 = 2.0 * (-wT * xR + xT * wR - yT * zR + zT * yR);
	float t1 = 2.0 * (-wT * yR + xT * zR + yT * wR - zT * xR);
	float t2 = 2.0 * (-wT * zR - xT * yR + yT * xR + zT * wR);

	mat4 weightedJointMatrix = mat4(
		1.0 - (2.0 * yR * yR) - (2.0 * zR * zR),
		(2.0 * xR * yR) + (2.0 * wR * zR),
		(2.0 * xR * zR) - (2.0 * wR * yR),
		0,

		(2.0 * xR * yR) - (2.0 * wR * zR),
		1.0 - (2.0 * xR * xR) - (2.0 * zR * zR),
		(2.0 * yR * zR) + (2.0 * wR * xR),
		0,

		(2.0 * xR * zR) + (2.0 * wR * yR),
		(2.0 * yR * zR) - (2.0 * wR * xR),
		1.0 - (2.0 * xR * xR) - (2.0 * yR * yR),
		0,

		t0,
		t1,
		t2,
		1);

	return weightedJointMatrix;
}

void main()
{
	mat4 SkinningMatrix = uUseDualQuaternions ?
		MakeDualQuaternionMatrix(vBoneWeights, ivec4(vBoneIndices)) :
		MakeSkinningMatrix(vBoneWeights, ivec4(vBoneIndices));

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
			fDebugColor.rb = vec2(vBoneWeights.y);
		}
		else if (int(vBoneIndices.z) == uDebugWeightSelector)
		{
			fDebugColor.rg = vec2(vBoneWeights.z);
		}
		else if (int(vBoneIndices.w) == uDebugWeightSelector)
		{
			fDebugColor.g = vBoneWeights.w;
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
