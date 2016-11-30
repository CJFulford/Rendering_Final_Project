#version 430 core

uniform mat4 modelview;
uniform mat4 projection;
uniform vec3 lightPosition;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

const float PI = 3.14159265359f;
const float PI2 = 2.f * PI;
const float PIo2 = PI / 2.f;

out vec3 V;
out vec3 N;
out vec3 L;
out vec2 UV;

// rotation matix's
mat3 rotateX(float a){return mat3(1.f, 0.f, 0.f, 0.f, cos(a), -sin(a), 0.f, sin(a), cos(a));}
mat3 rotateY(float a){return mat3(cos(a), 0.f, sin(a), 0.f, 1.f, 0.f, -sin(a), 0.f, cos(a));}
mat3 rotateZ(float a){return mat3(cos(a), -sin(a), 0.f, sin(a), cos(a), 0.f, 0.f, 0.f, 1.0);}

void main(void)
{
    vec3 positionModelSpace = vertex * rotateZ(PIo2) * rotateX(-PIo2);
	vec4 positionCameraSpace = modelview * vec4(positionModelSpace, 1.f);

	vec4 lightCameraSpace = modelview * vec4(lightPosition, 1.f);
	lightCameraSpace.xyz /= lightCameraSpace.w;
	
	mat3 normalMatrix = transpose(inverse(mat3(modelview)));

	V = normalize(positionCameraSpace.xyz);
	N = normalize(normalMatrix * normal);
	L = normalize(lightCameraSpace.xyz - positionCameraSpace.xyz);
	UV = uv;

    gl_Position = vec4(positionModelSpace, 1.f); 
}
