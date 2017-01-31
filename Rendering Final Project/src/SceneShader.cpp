#include "SceneShader.h"
#include "ShaderBuilder.h"
#include <glm\gtx\transform.hpp>

vec3 cam(0.f, 0.5f, 2.f);

void SceneShader::render(float time)
{
	projection = perspective(45.0f, aspectRatio, 0.01f, 100.0f);
	vec3 center(0.f, 0.3f, 0.f);
	vec3 up(0.f, 1.f, 0.f);
	modelview = lookAt(cam * zTranslation, center, up);

	mat4 rotationX = rotate(identity , xRot  * PI / 180.0f, vec3(1.f, 0.f, 0.f));
	mat4 rotationY = rotate(rotationX, yRot  * PI / 180.0f, vec3(0.f, 1.f, 0.f));
	modelview *= rotationY;

	//================================
	//render the fire
	glBindVertexArray(fireVertexArray);
	glUseProgram(fireProgram);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	texture.bind2DTexture(fireProgram, fireTexture, std::string("image"));
	glUniformMatrix4fv(glGetUniformLocation(fireProgram, "modelview"), 1, GL_FALSE, value_ptr(modelview));
	glUniformMatrix4fv(glGetUniformLocation(fireProgram, "projection"), 1, GL_FALSE, value_ptr(projection));

	glUniform1f(glGetUniformLocation(fireProgram, "time"), time);

	glDrawArrays(GL_LINE_STRIP, 0, fireGeneratedPoints);

	texture.unbind2DTexture();
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}

void SceneShader::setZTranslation(float z) { zTranslation = z; }
void SceneShader::setRotationX(float x) { xRot = x; }
void SceneShader::setRotationY(float y) { yRot = y; }
void SceneShader::buildShaders()
{
	fireProgram = compile_shaders("./shaders/fire.vert", "./shaders/fire.geom", "./shaders/fire.frag");
}
void SceneShader::startup() 
{ 
	buildShaders(); 
	createFireVertexBuffer(); 
}
SceneShader::~SceneShader() 
{
	glDeleteBuffers(1, &fireVertexBuffer);
	glDeleteBuffers(1, &fireVelocityBuffer);
	glDeleteVertexArrays(1, &fireUVBuffer);
	glDeleteVertexArrays(1, &fireVertexArray);
}
