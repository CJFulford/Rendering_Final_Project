/*
 * SceneShader.h
 *
 *  Created on: Nov 17, 2015
 *      Author: acarocha
 */

#ifndef SCENESHADER_H_
#define SCENESHADER_H_

#include "Shader.h"
#include "..\texture\texture.h"
#include "..\texture\lodepng.h"
#include <vector>

//#define GLM_FORCE_RADIANS

#include <GLFW/glfw3.h>
#include <TriMesh.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <cstdio>

const float PI = 3.14159265359f;
const float PI2 = 2.f * PI;
const float PIo2 = PI / 2.f;
const glm::mat4 identity(1.0f);

class SceneShader : public Shader
{
public:

	SceneShader();
	~SceneShader();

	void buildShaders();

	void startup ();
	void shutdown ();
	void render();

	void renderLogs();
	void renderSkybox();
	void renderFloor();

	void setAspectRatio(float ratio);
	void setZTranslation(float z);
	void setRotationX(float x );
	void setRotationY(float y);


private:

	/*methods*/

	trimesh::TriMesh* SceneShader::readMesh(std::string filename, std::vector<unsigned int> *triangleIndices);
	std::vector<glm::vec2> calculateSphereicalUVCoordinates(trimesh::TriMesh * mesh);
	std::vector<glm::vec2> SceneShader::calculateCylindricalUVCoordinates(trimesh::TriMesh* mesh);

	void createPlaneVertexBuffer();
	void createLogsVertexBuffer();
	void createSkyboxVertexBuffer();

	void SceneShader::passBasicUniforms(GLuint *program);

	GLuint loadTexture(std::string file_path);

	/*variables*/
	GLuint programLight;
	GLuint programFloor;
	GLuint programLogs;
	GLuint programSkybox;

	GLuint planeVertexArray;
	GLuint planeVertexBuffer;
	GLuint planeTextureBuffer;
	GLuint planeLogsUVBuffer;

	GLuint logsVertexArray;
	GLuint logsVertexBuffer;
	GLuint logsNormalBuffer;
	GLuint logsIndicesBuffer;
	GLuint logsInstanceBuffer;
	GLuint logsTextureBuffer;
	GLuint logsCylUVBuffer;

	GLuint skyboxVertexArray;
	GLuint skyboxVertexBuffer;
	GLuint skyboxNormalBuffer;
	GLuint skyboxIndicesBuffer;
	GLuint skyboxInstanceBuffer;
	GLuint skyboxTextureBuffer;
	GLuint skyboxCylUVBuffer;

	GLint mvUniform;
	GLuint projUniform;

	/* Matrices */
	glm::mat4 modelview;
	glm::mat4 projection;

	float zTranslation;
	float xRot;
	float yRot;
	float aspectRatio;

	// textures
	GLuint logsTexture;
	GLuint skyboxTexture;
	GLuint floorTexture;

	Texture texture;

	trimesh::TriMesh* logsMesh;
	std::vector<unsigned int> logsTriangleIndices;
	trimesh::TriMesh* skyboxMesh;
	std::vector<unsigned int> skyboxTriangleIndices;

	glm::vec3 lightPosition;
};

#endif /* SCENESHADER_H_ */
