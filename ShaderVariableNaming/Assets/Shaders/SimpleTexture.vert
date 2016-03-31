#version 150

in vec3 vPosition;
in vec2 vTexCoords;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec2 fTexCoord;
out vec2 fPosition;


void main()
{
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(vPosition, 1.0);
	fPosition = vPosition.xz;
	fTexCoord = vPosition.xz + vec2(0.5);
}
