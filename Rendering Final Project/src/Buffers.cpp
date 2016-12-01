#include "SceneShader.h"

std::string defaultTextureFile =	"textures/Black_and_White_grid.png";
std::string skyboxTextureFile =		"textures/skybox.png";
std::string floorTextureFile =		"textures/dirt.png";
std::string logsTextureFile =		"textures/embers.png";
std::string skybox =	"./models/skybox.ply";
std::string logs =		"./models/Logs.ply";

std::string fireTextureFile = "textures/fire_profile_texture.png";

float degToRad(float deg){return deg * PI / 180.f;}

std::vector<glm::vec2> SceneShader::calculateSphereicalUVCoordinates(trimesh::TriMesh* mesh)
{ 
	std::vector<glm::vec2> uv;
	float V;
	float U;
	float max_y;
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
	{
		float y = mesh->vertices[i][1];
		if (i == 0) max_y = y;
		else if (y > max_y) max_y = y;
	}
	bool toggle = false;
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
	{
		glm::vec3 vertex(mesh->vertices[i][0], mesh->vertices[i][1], mesh->vertices[i][2]);

		V = (PIo2 - (vertex.y / max_y)) / (PI / 4.f);
		U = (glm::atan(vertex.x, vertex.z) / PI2) + 0.5f;

		if (U < 0.25f) toggle = true;
		if (toggle && (U - 0.7f) > 0.f) U = 0.f;

		uv.push_back(glm::vec2(U, V));
	}
	return uv;
}

std::vector<glm::vec2> SceneShader::calculateCylindricalUVCoordinates(trimesh::TriMesh* mesh)
{
	std::vector<glm::vec2> uv;
	float U, V;
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
	{
		glm::vec3 vertex(mesh->vertices[i][0], mesh->vertices[i][1], mesh->vertices[i][2]);
		U = glm::atan(vertex.x, vertex.z) / PI2;
		V = vertex.y;

		uv.push_back(glm::vec2(U, V));
	}

	return uv;
}

void SceneShader::createLogsVertexBuffer()
{
	logsMesh = readMesh(logs, &logsTriangleIndices);
	logsTexture = loadTexture(logsTextureFile);

	glGenVertexArrays(1, &logsVertexArray);
	glBindVertexArray(logsVertexArray);

	glGenBuffers(1, &logsVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, logsVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, logsMesh->vertices.size() * sizeof(trimesh::point), logsMesh->vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &logsNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, logsNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, logsMesh->normals.size() * sizeof(trimesh::vec), logsMesh->normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	std::vector<glm::vec2> uvs = calculateCylindricalUVCoordinates(logsMesh);

	glGenBuffers(1, &logsCylUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, logsCylUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &logsIndicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, logsIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, logsTriangleIndices.size() * sizeof(unsigned int), logsTriangleIndices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void SceneShader::createSkyboxVertexBuffer()
{
	skyboxMesh = readMesh(skybox, &skyboxTriangleIndices);
	skyboxTexture = loadTexture(skyboxTextureFile);

	glGenVertexArrays(1, &skyboxVertexArray);
	glBindVertexArray(skyboxVertexArray);

	glGenBuffers(1, &skyboxVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, skyboxMesh->vertices.size() * sizeof(trimesh::point), skyboxMesh->vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &skyboxNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, skyboxMesh->normals.size() * sizeof(trimesh::vec), skyboxMesh->normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	std::vector<glm::vec2> uvs = calculateSphereicalUVCoordinates(skyboxMesh);

	glGenBuffers(1, &skyboxCylUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxCylUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &skyboxIndicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, skyboxTriangleIndices.size() * sizeof(unsigned int), skyboxTriangleIndices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void SceneShader::createFloorVertexBuffer()
{
	static const GLfloat floorGeometry[] =
	{
		-1.0f, 0.0f, -1.0f,
		-1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 1.0f,
	};
	static const GLfloat floorUVS[] =
	{
		0.f, 0.f,
		0.f, 1.f,
		1.f, 0.f,
		1.f, 1.f,
	};

	floorTexture = loadTexture(floorTextureFile);

	glGenVertexArrays(1, &floorVertexArray);
	glBindVertexArray(floorVertexArray);

	glGenBuffers(1, &floorVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, floorVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorGeometry), floorGeometry, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	//uvsPlane buffer
	glGenBuffers(1, &floorTextureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, floorTextureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorUVS), floorUVS, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void SceneShader::createFireVertexBuffer()
{
	fireTexture = loadTexture(floorTextureFile);
	// these are the coordinates for a point in the flame. x,y [-1, 1]. z [0,1]
	float tex_x;
	float tex_y;
	float tex_z;

	// calc uv and access texture like any other
	glm::vec2 uv;
	uv.x = sqrt((tex_x * tex_x) + (tex_y * tex_y));
	uv.y = tex_z;

	unsigned int n = 2; // number of control points
	unsigned int degree = 40; // unknown
	unsigned int i = 0; // B-spline that we are looking at


	float t[10];	//Knot vlaue
	float p[10];	//control point
	float N[10][10];	// B-spline basis Function
	float P[10];	// current control point

	float C;
	for (i = 0; i < n; i++)
	{
		C += N[i][degree] * P[i];
	}

	// there is a velocity function in the paper to sim movement
	// solving for P from P' in GPu is difficult and error prone, solving for P' from P in CPU is easy adn reliable
}

float bsplineBasis(float t, int  i, int n, float t_i[], float* N[])
{
	int p = 0;	//B-spline iterator



	int degree = 0; // unknown, possibly degree of curve

	if (i < degree) t_i[i] = 0.f;
	else if (degree < i && i < n) { t_i[i] = (i - degree) / (n - degree); }
	else t_i[i] = 1.f;

	// I am representing t as t[0] here as i do not know what t is.
	if (p == 0)
	{
		if (t_i[i] <= t < t_i[i + 1]) N[i][0] = 1.f;
		else N[i][0] = 0.f;
	}
	else
	{
		float a = ((t - t_i[i]) / (t_i[i + p] - t_i[i])) * N[i][p - 1];
		float b = ((t_i[i + p + 1] - t_i[0]) / (t_i[i + p + 1] - t_i[i + 1])) * N[i + 1][p - 1];
		N[i][p] = a + b;
	}
}


GLuint SceneShader::loadTexture(std::string file_path)
{
	std::vector<unsigned char> image;
	unsigned int imageWidth;
	unsigned int imageHeight;

	std::string imageFilename(file_path);
	unsigned int error = lodepng::decode(image, imageWidth, imageHeight, imageFilename.c_str());
	if (error) std::cout << "reading error" << error << ":" << lodepng_error_text(error) << std::endl;

	return  texture.create2DTexture(image, imageWidth, imageHeight);
}

trimesh::TriMesh* SceneShader::readMesh(std::string filename, std::vector<unsigned int>* triangleIndices)
{
	trimesh::TriMesh* mesh;

	mesh = trimesh::TriMesh::read(filename);

	mesh->need_bbox();
	mesh->need_faces();
	mesh->need_normals();
	mesh->need_bsphere();

	triangleIndices->clear();

	for (unsigned int i = 0; i < mesh->faces.size(); i++)
	{
		triangleIndices->push_back(mesh->faces[i][0]);
		triangleIndices->push_back(mesh->faces[i][1]);
		triangleIndices->push_back(mesh->faces[i][2]);
	}

	return mesh;
}