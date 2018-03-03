
#version 330

in vec3 vPosition;
in vec3 iPosition;
in vec3 iColor;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 fColor;


void main()
{
	fColor = iColor;
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(vPosition + iPosition, 1.0);
}
