#include "SceneShader.h"

float time = 0.f;
vec3 lightPosition = vec3(0.2f, 0.2f, 0.f);
vec3 cam(0.f, 0.5f, 2.f);

void SceneShader::renderFire()
{
	glBindVertexArray(fireVertexArray);
	glUseProgram(fireProgram);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	texture.bind2DTexture(fireProgram, fireTexture, std::string("image"));
	passBasicUniforms(&fireProgram);

	glUniform1f(glGetUniformLocation(fireProgram, "time"), time);

	int laps = 2;

	for (int i = 0; i < laps; i++)
	{
		glUniform1i(glGetUniformLocation(fireProgram, "lap"), i);
		glDrawArrays(GL_LINE_STRIP, 0, fireGeneratedPoints);
	}

	texture.unbind2DTexture();
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}

SceneShader::SceneShader() : Shader()
{
	fireProgram = 0;
	fireVertexArray = -1;
	zTranslation = 1.0;
	aspectRatio = 1.0;
	xRot = 0.0;
	yRot = 0.0;
}

void SceneShader::render()
{
	projection = perspective(45.0f, aspectRatio, 0.01f, 100.0f);
	vec3 center(0.f, 0.3f, 0.f);
	vec3 up(0.f, 1.f, 0.f);
	modelview = lookAt(cam * zTranslation, center, up);

	mat4 rotationX = rotate(identity , xRot  * PI / 180.0f, vec3(1.f, 0.f, 0.f));
	mat4 rotationY = rotate(rotationX, yRot  * PI / 180.0f, vec3(0.f, 1.f, 0.f));
	modelview *= rotationY;

	renderFire();
	
	time += 0.03f;
}

void SceneShader::passBasicUniforms(GLuint* program)
{
	glUniformMatrix4fv(glGetUniformLocation(*program, "modelview"), 1, GL_FALSE, value_ptr(modelview));
	glUniformMatrix4fv(glGetUniformLocation(*program, "projection"), 1, GL_FALSE, value_ptr(projection));
	glUniform3fv(glGetUniformLocation(*program, "lightPosition"), 1, value_ptr(lightPosition));
}

void SceneShader::shutdown()
{
	glDeleteBuffers(1, &fireVertexBuffer);
	glDeleteBuffers(1, &fireVelocityBuffer);
	glDeleteVertexArrays(1, &fireUVBuffer);
	glDeleteVertexArrays(1, &fireVertexArray);
}

void SceneShader::buildShaders()
{
	fireProgram = compile_shaders(	"./shaders/fire.vert",
									"./shaders/fire.geom",
									"./shaders/fire.frag");
}

void SceneShader::startup()
{
	buildShaders();
	createFireVertexBuffer();
}

void SceneShader::setZTranslation(float z) { zTranslation = z; }
void SceneShader::setAspectRatio(float ratio) { aspectRatio = ratio; }
void SceneShader::setRotationX(float x) { xRot = x; }
void SceneShader::setRotationY(float y) { yRot = y; }
SceneShader::~SceneShader() { shutdown(); }
