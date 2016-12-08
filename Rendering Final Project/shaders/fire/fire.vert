#version 430 core

layout (location = 0) in vec3 position;

uniform float time;

out vec3 vertex;

uniform mat4 modelview;
uniform mat4 projection;

void main (void)
{
    vertex = position.xyz;

	vertex.x += sin(time * 2.f) * 0.3f;
	vertex.z -= sin(time * 2.f) * 0.3f;

    gl_Position = projection *  modelview * vec4(vertex, 1.f);
}
