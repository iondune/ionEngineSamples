
#version 330

in vec3 vPosition;
in vec3 vColor;
in vec3 vNormal;
in vec3 vBarycentric;
in float vSelected;

uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec3 fColor;
out vec3 fNormal;
out vec3 fBarycentric;
out float fSelected;


void main()
{
	fColor = vColor;
	fNormal = (uNormalMatrix * vec4(vNormal, 0.0)).xyz;
	fBarycentric = vBarycentric;
	fSelected = vSelected;
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(vPosition, 1.0);
}
