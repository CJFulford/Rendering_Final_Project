/*  
 *	Created on: Nov 17, 2015
 *	Author: Cody Fulford
 *	Code wwas originally written by acarocha, however it has been heavily modified
 */

#ifndef SCENESHADER_H_
#define SCENESHADER_H_
#define GLM_FORCE_RADIANS

#include "Shader.h"
#include "..\texture\texture.h"
#include "..\texture\lodepng.h"
#include <vector>
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
const float err = 0.0001f;
const glm::mat4 identity(1.0f);

using namespace glm;

class SceneShader : public Shader
{
public:

	SceneShader();
	~SceneShader();

	void startup ();
	void shutdown ();
	void buildShaders();
	void render();

	void setAspectRatio(float ratio);
	void setZTranslation(float z);
	void setRotationX(float x );
	void setRotationY(float y);


private:
	// functions
	trimesh::TriMesh* readMesh(std::string filename, std::vector<unsigned int> *triangleIndices);
	std::vector<glm::vec2> calculateSphereicalUVCoordinates(trimesh::TriMesh * mesh);
	std::vector<glm::vec2> calculateCylindricalUVCoordinates(trimesh::TriMesh* mesh);

	void createFireVertexBuffer();

	void SceneShader::passBasicUniforms(GLuint *program);

	GLuint loadTexture(std::string file_path);

	// variables
	GLuint fireProgram;

	GLuint fireVertexArray;
	GLuint fireVertexBuffer;
	GLuint fireVelocityBuffer;
	GLuint fireUVBuffer;

	int fireGeneratedPoints = 0;

	GLuint fireTexture;

	mat4 modelview;
	mat4 projection;

	float zTranslation;
	float xRot;
	float yRot;
	float aspectRatio;

	Texture texture;
};

#endif /* SCENESHADER_H_ */
