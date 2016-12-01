#version 430 core

out vec4 color;

uniform sampler2D image;

uniform float time;

const float darkenFactor = 0.5f;
const float uvScale = 2.f;

in vec3 v;
in vec3 n;
in vec3 l;
in vec2 uv;



void main(void)
{
	vec2 UV;
	UV.x = mod((uv.x + time) * uvScale, 1.f);
	UV.y = mod(uv.y * uvScale, 1.f);
	color = texture(image, UV);
	color.x *= 0.7;
	color.y *= 0.4;
}


