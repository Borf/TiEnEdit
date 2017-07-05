#version 150

in vec3 a_position;
in vec2 a_texcoord;
in vec3 a_normal;

out vec2 texcoord;
out vec3 normal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;


void main()
{
	mat3 normalMatrix = mat3(viewMatrix * modelMatrix);
	normalMatrix = transpose(inverse(normalMatrix));

	normal = normalMatrix * a_normal;
	texcoord = a_texcoord;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position,1.0);
}