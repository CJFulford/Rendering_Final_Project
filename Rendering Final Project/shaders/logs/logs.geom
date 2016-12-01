#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 modelview;
uniform mat4 projection;

in vec3 V[];	
in vec3 N[];
in vec3 L[];
in vec2 UV[];

out vec3 v;
out vec3 n;
out vec3 l;
out vec2 uv;

const float PI = 3.14159265559;
const float PI2 = PI * 2.f;
const float numOfLogs = 6.f;

// rotation matix's
mat3 rotateX(float a){return mat3(1.f, 0.f, 0.f, 0.f, cos(a), -sin(a), 0.f, sin(a), cos(a));}
mat3 rotateY(float a){return mat3(cos(a), 0.f, sin(a), 0.f, 1.f, 0.f, -sin(a), 0.f, cos(a));}
mat3 rotateZ(float a){return mat3(cos(a), -sin(a), 0.f, sin(a), cos(a), 0.f, 0.f, 0.f, 1.0);}

void main(void)
{
	for (float rad = 0; rad < PI2; rad += PI2 / numOfLogs)
	{
		for(int i = 0; i < gl_in.length(); i++)
		{
			vec3 position = gl_in[i].gl_Position.xyz;

			v = V[i];
			n = N[i];
			l = L[i];

			uv = UV[i];
			uv.x += mod(rad / (PI / 10.f), 1.f);

			position *= rotateY(rad);

			gl_Position = projection * modelview * vec4(position, 1.f);

			EmitVertex();
		}
		EndPrimitive();
	}
}