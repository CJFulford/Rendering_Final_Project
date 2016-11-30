#version 430 core

uniform mat4 modelview;
uniform mat4 projection;
uniform vec3 lightPosition;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 cylUV;

out vec3 N;
out vec3 L;
out vec3 V;
out vec2 UV;


void main(void)
{
    vec3 positionModelSpace = vertex;
	positionModelSpace.xz *= 0.5f;
    vec4 positionCameraSpace = modelview * vec4(positionModelSpace, 1.f);
	vec4 lightCameraSpace = modelview * vec4(lightPosition, 1.f);
	lightCameraSpace.xyz /= lightCameraSpace.w;
	mat3 normalMatrix = transpose(inverse(mat3(modelview)));

	N = normalize(normalMatrix*normal);
	L = normalize(lightCameraSpace.xyz - positionCameraSpace.xyz);
	V = normalize(-positionCameraSpace.xyz);
	UV = cylUV;

    gl_Position = projection * positionCameraSpace; 
}
