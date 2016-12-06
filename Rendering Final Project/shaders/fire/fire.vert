#version 430 core

layout (location = 0) in vec3 position;

out vec3 vertex;

uniform mat4 modelview;
uniform mat4 projection;

void main (void)
{
    vertex = position.xyz;
    gl_Position = projection *  modelview * vec4(position.xyz, 1.f);
}
