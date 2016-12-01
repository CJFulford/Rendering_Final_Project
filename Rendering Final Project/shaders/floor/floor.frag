#version 430 core

const float PI = 3.14159265559;
const float PI2 = PI * 2.f;
const float numOfLogs = 6.f;
const float uvScale = 2.f;

out vec4 color;

uniform mat4 modelview;
uniform sampler2D image;
uniform mat4 projection;

float phongExp = 10.f;

in vec3 V;
in vec3 N;
in vec3 L;
in vec2 UV;

float imageScale = 10.f;



// rotation matix's
mat3 rotateX(float a){return mat3(1.f, 0.f, 0.f, 0.f, cos(a), -sin(a), 0.f, sin(a), cos(a));}
mat3 rotateY(float a){return mat3(cos(a), 0.f, sin(a), 0.f, 1.f, 0.f, -sin(a), 0.f, cos(a));}
mat3 rotateZ(float a){return mat3(cos(a), -sin(a), 0.f, sin(a), cos(a), 0.f, 0.f, 0.f, 1.0);}


void main (void)
{

	vec4 col;

	for (float rad = 0; rad < PI2; rad += PI2 / numOfLogs)
	{
		vec3 l = L * rotateY(rad);
		col += texture(image, UV * imageScale) * max(0.f, dot(N, normalize(l - V)));
	}

	color = col;
	color.xyz *= 0.15f;
}
