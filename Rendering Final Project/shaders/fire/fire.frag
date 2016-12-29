#version 430 core

out vec4 color;

uniform sampler2D image;

in vec3 vertex;
in vec3 col;

void main (void)
{

	// these are the coordinates for a point in the flame. x,y [-1, 1]. z [0,1]
	// do the texture work in the shader
	// calc uv and access texture like any other
	vec2 uv;
	uv.x = sqrt((vertex.x * vertex.x) + (vertex.y * vertex.y));
	uv.y = vertex.z;

	//color = texture(image, uv); // just leave as green for default
	color = vec4(col, 1.f);
}
