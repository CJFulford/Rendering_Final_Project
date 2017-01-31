/*  
 *	Created on: Nov 17, 2015
 *	Author: Cody Fulford
 */
#define GLM_FORCE_RADIANS

#include "..\texture\texture.h"
#include "..\texture\lodepng.h"
#include  "ToolsHeader.h"
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp>

const int SCREEN_WIDTH = 700, SCREEN_HEIGHT = SCREEN_WIDTH;

const float PI = 3.14159265359f,
			twoPI = 2.f * PI,
			halfPI = PI / 2.f,
			err = 0.0001f,
			aspectRatio = SCREEN_WIDTH / SCREEN_HEIGHT;

const glm::mat4 identity(1.0f);

using namespace glm;

class SceneShader
{
public:

	~SceneShader();

	void startup ();
	void buildShaders();
	void render(float time);

	void setZTranslation(float z);
	void setRotationX(float x );
	void setRotationY(float y);


private:
	void createFireVertexBuffer();
	GLuint loadTexture(std::string file_path);

	GLuint fireProgram = 0,
			fireVertexArray = -1,
			fireVertexBuffer,
			fireVelocityBuffer,
			fireUVBuffer,
			fireTexture;

	int fireGeneratedPoints = 0;

	mat4 modelview, 
		projection;

	float zTranslation = 1.f;
	float xRot = 0.f;
	float yRot = 0.f;

	Texture texture;
};
