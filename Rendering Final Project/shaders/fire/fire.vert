#version 430 core

layout (location = 0) in vec3 position;=

out vec3 V;
out vec3 N;
out vec3 L;
out vec2 UV;

uniform vec3 lightPosition;
uniform mat4 modelview;
uniform mat4 projection;


mat3 rotateX(float a){return mat3(1.f, 0.f, 0.f, 0.f, cos(a), -sin(a), 0.f, sin(a), cos(a));}
mat3 rotateY(float a){return mat3(cos(a), 0.f, sin(a), 0.f, 1.f, 0.f, -sin(a), 0.f, cos(a));}
mat3 rotateZ(float a){return mat3(cos(a), -sin(a), 0.f, sin(a), cos(a), 0.f, 0.f, 0.f, 1.f);}

void main (void)
{
	
    vec3 lightModelSpace = lightPosition;
    
	vec4 positionModelSpace = vec4(position.xyz, 1.f);
	
	vec4 positionCameraSpace = modelview * positionModelSpace;

    mat3 normalMatrix = mat3(transpose(inverse(modelview)));

    V = normalize(positionModelSpace.xyz);
    N = normalize(normalMatrix * vec3(0.f, 1.f, 0.f));
    L = normalize(lightModelSpace.xyz);
    UV = uv;

    gl_Position = projection *  positionCameraSpace;

}
