#version 430 core

out vec4 color;

uniform sampler2D image;

in vec3 N;
in vec3 L;
in vec3 V;
in vec2 UV;

void main(void)
{	
	color = texture(image, UV);
	color.xyz *= 0.15f;
}
