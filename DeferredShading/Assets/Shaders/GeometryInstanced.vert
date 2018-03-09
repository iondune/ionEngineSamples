
#version 330

in vec3 vPosition;
in vec3 vNormal;

in vec3 iPosition;
in vec3 iColor;

uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 fColor;


void main()
{
	fColor = iColor;

	mat4 PositionMatrix = mat4(1.0);
	PositionMatrix[3].xyz = iPosition;

	gl_Position = uProjectionMatrix * uViewMatrix * PositionMatrix * uModelMatrix * vec4(vPosition, 1.0);
	// gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(vPosition, 1.0);
}
