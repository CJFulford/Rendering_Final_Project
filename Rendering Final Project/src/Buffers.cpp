#include "SceneShader.h"
#include <omp.h>

std::string skyboxTextureFile =		"textures/skybox.png";
std::string floorTextureFile =		"textures/dirt.png";
std::string logsTextureFile =		"textures/embers.png";
std::string fireTextureFile =		"textures/fire_profile_texture.png";
std::string skybox =	"./models/skybox.ply";
std::string logs =		"./models/Logs.ply";

float spline(float knot, float knots[], int numOfKnots, int i, int p)
{
	float	t = knot,
			ti = knots[i],
			ti1 = knots[i + 1],
			tip = knots[i + p],
			tip1 = knots[i + p + 1];

	if (p == 0.f)
		if (ti <= t && t < ti1) return 1.f;
		else					return 0.f;
	else
	{
		float	ratio1	= 0.f, 
				ratio2	= 0.f,
				den1	= tip  - ti,
				den2	= tip1 - ti1;

		// if a denominator = 0 (nan return), simply make that ratio 0.
		if (den1 == 0.f) ratio1 = 0.f;
		else
		{
			ratio1 = (t - ti) / den1;
			ratio1 *= spline(knot, knots, numOfKnots, i, p - 1);
		}


		if (den2 == 0.f) ratio2 = 0.f;
		else
		{
			ratio2 = (tip1 - t) / den2;
			ratio2 *= spline(knot, knots, numOfKnots, i + 1, p - 1);
		}
		return ratio1 + ratio2;
	}
}

void SceneShader::createFireVertexBuffer()
{
	vec3	fireBase = vec3(0.f, 0.f, 0.f),
			fireTop	 = vec3(0.f, 1.f, 0.f);


	const int	totalControlPoints	= 10,					// desired nmumber of control points
				n					= totalControlPoints,
				degree				= 3,					// 3rd degree curve (why not?)
				numOfKnots			= (totalControlPoints - 1) + degree + 2;	// number of knot values

	float knots[numOfKnots];

	// generate all of the control points in a straight line from the base to the top
	vec3 controlPoints[totalControlPoints];
	// totalControlPoints-1 so that fireTop is one of the control points
	float controlPointStep = 1.f / ((float)totalControlPoints - 1.f); 
	#pragma omp parallel for
	for (int i = 0; i < totalControlPoints; i++)
		controlPoints[i] = fireBase + (float)i*controlPointStep * fireTop;

	// generate the knot points. the if an the else keep the spline closed. Knot spacing is uniform, [0,1]
	// equation from fire paper

	// array holding the final position of the knots
	vec3 C[numOfKnots];

	// C[i] is there so thatwe can skip some loops in the next for loop
	// since, under these conditions, C[i]= either the fire base or the fire top, i saved some processing and did it this way
	#pragma omp parallel for
	for (int i = 0; i < numOfKnots; i++)
	{
		if (i <= degree)
		{
			knots[i] = 0.f; 
			C[i] = fireBase;
		}
		else if (i < n)
			knots[i] = (float)(i - degree) / (float)(n - degree);
		else
		{
			knots[i] = 1.f;
			C[i] = fireTop;
		}
	}

	#pragma omp parallel for
	for (int knot = degree + 1; knot < n; knot++)
		for (int i = 0; i < numOfKnots; i++)
			C[knot] += controlPoints[i] * spline(knots[knot], knots, numOfKnots, i, degree);



	fireTexture = loadTexture(fireTextureFile);

	glGenVertexArrays(1, &fireVertexArray);
	glBindVertexArray(fireVertexArray);

	glGenBuffers(1, &fireVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(C), C, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
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

	std::vector<vec2> uvs = calculateCylindricalUVCoordinates(logsMesh);

	glGenBuffers(1, &logsCylUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, logsCylUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), uvs.data(), GL_STATIC_DRAW);
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

	std::vector<vec2> uvs = calculateSphereicalUVCoordinates(skyboxMesh);

	glGenBuffers(1, &skyboxCylUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxCylUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), uvs.data(), GL_STATIC_DRAW);
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

std::vector<vec2> SceneShader::calculateSphereicalUVCoordinates(trimesh::TriMesh* mesh)
{
	std::vector<vec2> uv;
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
		vec3 vertex(mesh->vertices[i][0], mesh->vertices[i][1], mesh->vertices[i][2]);

		V = (PIo2 - (vertex.y / max_y)) / (PI / 4.f);
		U = (atan(vertex.x, vertex.z) / PI2) + 0.5f;

		if (U < 0.25f) toggle = true;
		if (toggle && (U - 0.7f) > 0.f) U = 0.f;

		uv.push_back(vec2(U, V));
	}
	return uv;
}

std::vector<vec2> SceneShader::calculateCylindricalUVCoordinates(trimesh::TriMesh* mesh)
{
	std::vector<vec2> uv;
	float U, V;
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
	{
		vec3 vertex(mesh->vertices[i][0], mesh->vertices[i][1], mesh->vertices[i][2]);
		U = atan(vertex.x, vertex.z) / PI2;
		V = vertex.y;

		uv.push_back(vec2(U, V));
	}

	return uv;
}
