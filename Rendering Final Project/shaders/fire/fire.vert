#version 430 core

layout (location = 0) in vec3 position;

out vec3 vertex;

uniform mat4 modelview;
uniform mat4 projection;

uniform float time;

void main (void)
{
    vertex = position.xyz;

	if (vertex.y != 0)
	{
		// generate a spiral movement pattern, this will be replaced
		// but is a proof of concept for now
		vertex.x += cos(time * (1.f / (1.f - vertex.y)));
		vertex.z += sin(time * (1.f / (1.f - vertex.y)));

		// scale the spiral to adjust the radius of the fire.
		// base it on the y dimension so that the base of the fire has a larger radius
		vertex.xz *= (1.f - vertex.y) * 0.1f;
	}

    gl_Position = projection *  modelview * vec4(vertex, 1.f);
}
