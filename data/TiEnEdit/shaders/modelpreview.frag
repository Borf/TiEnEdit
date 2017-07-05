#version 150

uniform sampler2D s_texture;
uniform vec4 color;
uniform float textureFactor;

in vec2 texcoord;
in vec3 normal;
out vec4 fragColor;

void main()
{
	float lighting = 0.5 + 0.5 * dot(normal, vec3(0,0,1));

	vec4 color = mix(color, texture2D( s_texture, texcoord ), textureFactor);

	fragColor.rgb = lighting * color.rgb;
	fragColor.a = color.a;
}
