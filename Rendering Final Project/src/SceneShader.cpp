#include "SceneShader.h"

float time = 0.f;

void SceneShader::renderLogs()
{
	glBindVertexArray(logsVertexArray);
	glUseProgram(programLogs);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	texture.bind2DTexture(programLogs, logsTexture, std::string("image"));
	passBasicUniforms(&programLogs);
	
	glUniform1f(glGetUniformLocation(programLogs, "time"), time);

	glDrawElements(GL_TRIANGLES, logsMesh->faces.size() * 3, GL_UNSIGNED_INT, 0);
	texture.unbind2DTexture();
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}

void SceneShader::renderSkybox()
{
	glBindVertexArray(skyboxVertexArray);
	glUseProgram(programSkybox);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	texture.bind2DTexture(programSkybox, skyboxTexture, std::string("image"));
	passBasicUniforms(&programSkybox);

	glDrawElements(GL_TRIANGLES, skyboxMesh->faces.size() * 3, GL_UNSIGNED_INT, 0);
	texture.unbind2DTexture();
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}

void SceneShader::renderFloor()
{
	glBindVertexArray(planeVertexArray);
	glUseProgram(programFloor);
	texture.bind2DTexture(programFloor, floorTexture, std::string("image"));
	passBasicUniforms(&programFloor);

	texture.bind2DTexture(programFloor, logsTexture, std::string("imagelog"));
	glUniform1f(glGetUniformLocation(programLogs, "time"), time);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	texture.unbind2DTexture();
	glBindVertexArray(0);
}

SceneShader::SceneShader() : Shader()
{
	programFloor = 0;
	programLogs = 0;
	programSkybox = 0;
	planeVertexArray = -1;
	logsVertexArray = -1;
	skyboxVertexArray = -1;
	mvUniform = -1;
	projUniform = -1;
	zTranslation = 1.0;
	aspectRatio = 1.0;
	xRot = 0.0;
	yRot = 0.0;
	lightPosition = glm::vec3(0.2f, 0.2f, 0.f);
}

void SceneShader::render()
{
	projection = glm::perspective(45.0f, aspectRatio, 0.01f, 100.0f);

	glm::vec3 eye(0.f, 0.3f, 2.f);
	glm::vec3 center(0.f, 0.f, 0.f);
	glm::vec3 up(0.f, 1.f, 0.f);
	eye *= zTranslation;
	modelview = glm::lookAt(eye, center, up);

	glm::mat4 rotationY = glm::rotate(identity , xRot  * PI / 180.0f, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 rotationX = glm::rotate(rotationY, yRot  * PI / 180.0f, glm::vec3(0.f, 1.f, 0.f));
	modelview *= rotationX;

	renderFloor();
	renderLogs();
	renderSkybox();
	
	time += 0.0003f;
}

void SceneShader::buildShaders()
{

	programLogs		= compile_shaders(	"./shaders/logs/logs.vert",
										"./shaders/logs/logs.geom",
										"./shaders/logs/logs.frag");

	programFloor	= compile_shaders(	"./shaders/floor/floor.vert",
										"./shaders/floor/floor.frag");

	programSkybox	= compile_shaders(	"./shaders/skybox/skybox.vert",	
										"./shaders/skybox/skybox.frag");
}

void SceneShader::passBasicUniforms(GLuint* program)
{
	glUniformMatrix4fv(glGetUniformLocation(*program, "modelview"), 1, GL_FALSE, glm::value_ptr(modelview));
	glUniformMatrix4fv(glGetUniformLocation(*program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3fv(glGetUniformLocation(*program, "lightPosition"), 1, glm::value_ptr(lightPosition));
}

void SceneShader::shutdown()
{
	glDeleteBuffers(1, &logsVertexBuffer);
	glDeleteBuffers(1, &logsNormalBuffer);
	glDeleteBuffers(1, &logsIndicesBuffer);
	glDeleteVertexArrays(1, &logsVertexArray); 
	
	glDeleteBuffers(1, &skyboxVertexBuffer);
	glDeleteBuffers(1, &skyboxNormalBuffer);
	glDeleteBuffers(1, &skyboxIndicesBuffer);
	glDeleteVertexArrays(1, &skyboxVertexArray);
	
	glDeleteVertexArrays(1, &planeVertexArray);
}

void SceneShader::startup()
{
	buildShaders();
	createLogsVertexBuffer();
	createPlaneVertexBuffer();
	createSkyboxVertexBuffer();
}

void SceneShader::setZTranslation(float z) { zTranslation = z; }
void SceneShader::setAspectRatio(float ratio) { aspectRatio = ratio; }
void SceneShader::setRotationX(float x) { xRot = x; }
void SceneShader::setRotationY(float y) { yRot = y; }
SceneShader::~SceneShader() { shutdown(); }
