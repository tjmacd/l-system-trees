/*
 *  Simple vertex shader for example five
 */

in vec4 vPosition;
in vec3 vNormal;
uniform mat4 model;
uniform mat4 viewPerspective;
out vec3 normal;
out vec3 position;

void main() {

	gl_Position = viewPerspective * model * vPosition;
	position = vPosition.xyz;
	normal = vNormal;
}