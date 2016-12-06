#version 430 core

out vec4 color;

uniform mat4 modelview;
uniform mat4 projection;
uniform sampler2D image;


in vec3 V;
in vec3 N;
in vec3 L;
in vec2 UV;


// rotation matix's
mat3 rotateX(float a){return mat3(1.f, 0.f, 0.f, 0.f, cos(a), -sin(a), 0.f, sin(a), cos(a));}
mat3 rotateY(float a){return mat3(cos(a), 0.f, sin(a), 0.f, 1.f, 0.f, -sin(a), 0.f, cos(a));}
mat3 rotateZ(float a){return mat3(cos(a), -sin(a), 0.f, sin(a), cos(a), 0.f, 0.f, 0.f, 1.0);}


void main (void)
{

	// these are the coordinates for a point in the flame. x,y [-1, 1]. z [0,1]
	// do the texture work in the shader
	
	float tex_x;
	float tex_y;
	float tex_z;

	// calc uv and access texture like any other
	glm::vec2 uv;
	uv.x = sqrt((tex_x * tex_x) + (tex_y * tex_y));
	uv.y = tex_z;

	color = vec4(1.f, 0.f, 0.f, 1.f); // just leave as red for default
}
