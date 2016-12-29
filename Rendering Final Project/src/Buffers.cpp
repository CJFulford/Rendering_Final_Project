#include "SceneShader.h"
#include <omp.h>

std::string skyboxTextureFile =		"textures/skybox.png";
std::string floorTextureFile =		"textures/dirt.png";
std::string logsTextureFile =		"textures/embers.png";
std::string fireTextureFile =		"textures/fire_profile_texture.png";
std::string skybox =	"./models/skybox.ply";
std::string logs =		"./models/Logs.ply";

void printVec3(vec3 v) { std::cout << v.x << "\t" << v.y << "\t" << v.z << std::endl; }

float spline(float knot, float knots[], int numOfKnots, int i, int p)
{
	float	t = knot,
			ti = knots[i],
			ti1 = knots[i + 1],
			tip = knots[i + p],
			tip1 = knots[i + p + 1];

	if (p < err)
		if (ti <= t && t < ti1) return 1.f;
		else					return 0.f;
	else
	{
		float	ratio1	= 0.f, 
				ratio2	= 0.f,
				den1	= tip  - ti,
				den2	= tip1 - ti1;

		// if a denominator = 0 (nan return), simply make that ratio 0.
		if (den1 < err) ratio1 = 0.f;
		else
		{
			ratio1 = (t - ti) / den1;
			ratio1 *= spline(knot, knots, numOfKnots, i, p - 1);
		}


		if (den2 < err) ratio2 = 0.f;
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
	// 10
	vec3 controlPoints[] =
	{ 
		vec3(0.f, 0.f, 0.f),
		vec3(0.05f, 0.1f, 0.f),
		vec3(0.1f, 0.2f, 0.f),
		vec3(0.15f, 0.35f, 0.f),
		vec3(0.2f, 0.4f, 0.f),
		vec3(0.3f, 0.425f, 0.f),
		vec3(0.35f, 0.45f, 0.f),
		vec3(0.4f, 0.5f, 0.f),
		vec3(0.45f, 0.6f, 0.f),
		vec3(0.5f, 0.7f, 0.f) 
	};

	vec3 fireBase(0.f, 0.0f, 0.f);
	vec3 fireTop(0.f, 0.6f, 0.f);


	const int	totalControlPoints	= sizeof(controlPoints) / sizeof(controlPoints[0]),	// desired nmumber of control points
				n					= totalControlPoints,
				degree				= 3,			// 3rd degree curve (why not?)
				numOfKnots			= (totalControlPoints - 1) + degree + 2;	// number of knot values

	float knots[numOfKnots];

	#define line
	#ifdef line
		float controlPointStep = 1.f / ((float)totalControlPoints - 1.f); // totalControlPoints-1 so that fireTop is one of the control points
		for (int i = 1; i < totalControlPoints; i++)
			controlPoints[i] = fireBase + (float)i * controlPointStep * fireTop;
	#endif
	
	// generate the knot points. the if an the else keep the spline closed. Knot spacing is uniform, [0,1]
	// equation from fire paper

	// array holding the final position of the knots
	vec3 C[numOfKnots];

	// C[i] is there so thatwe can skip some loops in the next for loop
	for (int i = 0; i < numOfKnots; i++)
	{
		if (i <= degree)
			knots[i] = 0.f; 
		else if (i < n)
		{
			knots[i] = (float)(i - degree) / (float)(n - degree);
			if (knots[i] < err) knots[i] = 0.f;
		}
		else
			knots[i] = 1.f;
	}


	for (int knot = 0; knot < n; knot++)
		for (int i = 0; i < numOfKnots; i++)
			C[knot] += controlPoints[i] * spline(knots[knot], knots, numOfKnots, i, degree);

	// b/c otherwise, the last points will be all 0
	for (int i = numOfKnots - degree - 1; i < numOfKnots; i++)
		C[i] = fireTop;

	// view for error checking
	for (int knot = 0; knot < numOfKnots; knot++)
		printVec3(C[knot]);

	vec3 velocity[numOfKnots];	// velocity vector for each knot point
	for (int i = 0; i < numOfKnots; i++)
	{
		if (i == numOfKnots - 1)
			velocity[i] = C[i - 1] - C[i];
		velocity[i] = C[i] - C[i + 1];
	}

	fireGeneratedPoints = sizeof(C) / sizeof(C[0]);

	fireTexture = loadTexture(fireTextureFile);

	glGenVertexArrays(1, &fireVertexArray);
	glBindVertexArray(fireVertexArray);

	glGenBuffers(1, &fireVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(C), C, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &fireVelocityBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireVelocityBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(velocity), velocity, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);


	// fuck you code. do it until you do it right
	/*
		      __    
		     |  |
			 |  |
		   __|  |_____
		  |  |  |  |  |
		  |  |  |  |  |
		  |           |
		   \         /
		    \_______/
	*/
	bool redo = false;
	for (int i = 0; i < numOfKnots; i++)
		if (isnan(C[i].x) || isnan(C[i].y) || isnan(C[i].z))
		{
			redo = true;
			break;
		}
	if (redo) createFireVertexBuffer();

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
