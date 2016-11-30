#version 430 core

uniform mat4 modelview;
uniform mat4 projection;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec3 V;
out vec3 N;
out vec3 L;
out vec2 UV;

uniform vec3 lightPosition;

float floorScale = 15f;
float floorVertical = -0.08f;

const float PI = 3.14159265359f;
const float PI2 = 2.f * PI;
const float PIo2 = PI / 2.f;

mat3 rotateX(float a){return mat3(1.f, 0.f, 0.f, 0.f, cos(a), -sin(a), 0.f, sin(a), cos(a));}
mat3 rotateY(float a){return mat3(cos(a), 0.f, sin(a), 0.f, 1.f, 0.f, -sin(a), 0.f, cos(a));}
mat3 rotateZ(float a){return mat3(cos(a), -sin(a), 0.f, sin(a), cos(a), 0.f, 0.f, 0.f, 1.f);}

void main (void)
{
	
    vec3 lightModelSpace = lightPosition * rotateZ(PIo2) * rotateX(-PIo2);
	vec4 lightCameraSpace = modelview * vec4(lightModelSpace, 1.f);
	lightCameraSpace.xyz /= lightCameraSpace.w;
    
	vec4 positionModelSpace = vec4(	position.x * floorScale, 
									position.y + floorVertical, 
									position.z * floorScale, 
									1.f);
	
	vec4 positionCameraSpace =modelview * positionModelSpace;

    mat3 normalMatrix = mat3(transpose(inverse(modelview)));

    V = positionModelSpace.xyz;
    N = normalize(normalMatrix * vec3(0.f, 1.f, 0.f));
    L = lightCameraSpace.xyz;
    UV = uv;

    gl_Position = projection *  positionCameraSpace;

}
