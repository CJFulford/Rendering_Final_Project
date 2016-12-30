#include "SceneShader.h"

float time = 0.f;
vec3 lightPosition = vec3(0.2f, 0.2f, 0.f);
vec3 cam(0.f, 0.5f, 2.f);

void SceneShader::renderFire()
{
	glBindVertexArray(fireVertexArray);
	glUseProgram(fireProgram);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	texture.bind2DTexture(fireProgram, fireTexture, std::string("image"));
	passBasicUniforms(&fireProgram);

	glUniform1f(glGetUniformLocation(fireProgram, "time"), time);

	glDrawArrays(GL_LINE_STRIP, 0, fireGeneratedPoints);

	texture.unbind2DTexture();
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}

void SceneShader::renderLogs()
{
	glBindVertexArray(logsVertexArray);
	glUseProgram(logsProgram);

	texture.bind2DTexture(logsProgram, logsTexture, std::string("image"));
	passBasicUniforms(&logsProgram);
	
	glUniform1f(glGetUniformLocation(logsProgram, "time"), time);

	glDrawElements(GL_TRIANGLES, logsMesh->faces.size() * 3, GL_UNSIGNED_INT, 0);
	texture.unbind2DTexture();
	glBindVertexArray(0);
}

void SceneShader::renderSkybox()
{
	glBindVertexArray(skyboxVertexArray);
	glUseProgram(skyboxProgram);

	texture.bind2DTexture(skyboxProgram, skyboxTexture, std::string("image"));
	passBasicUniforms(&skyboxProgram);

	glDrawElements(GL_TRIANGLES, skyboxMesh->faces.size() * 3, GL_UNSIGNED_INT, 0);
	texture.unbind2DTexture();
	glBindVertexArray(0);
}

void SceneShader::renderFloor()
{
	glBindVertexArray(floorVertexArray);
	glUseProgram(floorProgram);

	texture.bind2DTexture(floorProgram, floorTexture, std::string("image"));
	passBasicUniforms(&floorProgram);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	texture.unbind2DTexture();
	glBindVertexArray(0);
}

SceneShader::SceneShader() : Shader()
{
	floorProgram = 0;
	logsProgram = 0;
	skyboxProgram = 0;
	floorVertexArray = -1;
	logsVertexArray = -1;
	skyboxVertexArray = -1;
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
	//renderFloor();
	//renderLogs();
	//renderSkybox();
	
	time += 0.03f;
}

void SceneShader::buildShaders()
{
	fireProgram		= compile_shaders(	"./shaders/fire/fire.vert",
										"./shaders/fire/fire.geom",
										"./shaders/fire/fire.frag");

	logsProgram		= compile_shaders(	"./shaders/logs/logs.vert",
										"./shaders/logs/logs.geom",
										"./shaders/logs/logs.frag");

	floorProgram	= compile_shaders(	"./shaders/floor/floor.vert",
										"./shaders/floor/floor.frag");

	skyboxProgram	= compile_shaders(	"./shaders/skybox/skybox.vert",	
										"./shaders/skybox/skybox.frag");
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

	glDeleteBuffers(1, &logsVertexBuffer);
	glDeleteBuffers(1, &logsNormalBuffer);
	glDeleteBuffers(1, &logsIndicesBuffer);
	glDeleteVertexArrays(1, &logsVertexArray); 
	
	glDeleteBuffers(1, &skyboxVertexBuffer);
	glDeleteBuffers(1, &skyboxNormalBuffer);
	glDeleteBuffers(1, &skyboxIndicesBuffer);
	glDeleteVertexArrays(1, &skyboxVertexArray);
	
	glDeleteVertexArrays(1, &floorVertexArray);
}

void SceneShader::startup()
{
	buildShaders();
	createLogsVertexBuffer();
	createFloorVertexBuffer();
	createSkyboxVertexBuffer();
	createFireVertexBuffer();
}

void SceneShader::setZTranslation(float z) { zTranslation = z; }
void SceneShader::setAspectRatio(float ratio) { aspectRatio = ratio; }
void SceneShader::setRotationX(float x) { xRot = x; }
void SceneShader::setRotationY(float y) { yRot = y; }
SceneShader::~SceneShader() { shutdown(); }
