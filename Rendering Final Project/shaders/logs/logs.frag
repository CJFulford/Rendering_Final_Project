#version 430 core

out vec4 color;

uniform sampler2D image;

uniform float time;

in vec3 v;
in vec3 n;
in vec3 l;
in vec2 uv;

void main(void)
{	
	vec2 UV;
	UV.x = mod(uv.x + time, 1.f);
	UV.y = mod(uv.y, 1.f);
	color = texture(image, UV);
}


