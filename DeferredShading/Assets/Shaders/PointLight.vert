
#version 330

in vec3 vPosition;
in vec3 iPosition;
in vec3 iColor;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

uniform float uLightRadius;

out vec2 fTexCoords;
out vec3 fPosition;
out vec3 fColor;

void main()
{
	gl_Position = uProjectionMatrix * uViewMatrix * vec4(vPosition * uLightRadius + iPosition, 1.0);
	fTexCoords = gl_Position.xy / gl_Position.w / 2.0 + vec2(0.5);
	fPosition = iPosition;
	fColor = iColor;
}
