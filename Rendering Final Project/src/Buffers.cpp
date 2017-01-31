#include "SceneShader.h"
#include <omp.h>

std::string fireTextureFile = "textures/fire_profile_texture.png";

void printVec3(vec3 v) 
{ 
	std::cout << v.x << "\t" << v.y << "\t" << v.z << std::endl; 
}

//Rodrigues' rotation formula
vec3 rotAny(vec3 vector, vec3 axis, float angle) 
{ 
	return vec3((vector * cos(angle)) + 
		(cross(axis, vector) * sin(angle)) + 
		(axis * dot(axis, vector) * (1.f - cos(angle)))); 
}

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
		float	ratio1 = 0.f,
			ratio2 = 0.f,
			den1 = tip - ti,
			den2 = tip1 - ti1;

		// if a denominator = 0 (nan return), simply make that ratio 0.
		if (abs(den1) < err) ratio1 = 0.f;
		else
		{
			ratio1 = (t - ti) / den1;
			ratio1 *= spline(knot, knots, numOfKnots, i, p - 1);
		}


		if (abs(den2) < err) ratio2 = 0.f;
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
	const int	totalControlPoints = 100,
		m = totalControlPoints - 1,
		degree = 3,
		order = degree + 1,
		numOfKnots = (totalControlPoints - 1) + degree + 2;

	float	knots[numOfKnots],
			UV[numOfKnots];
	
	vec3	points[numOfKnots],
			controlPoints[totalControlPoints],
			fireBase(0.f, 0.f, 0.f),
			fireTop(0.f, 1.f, 0.f);

	float controlPointStep = 1.f / (m);

	// generate linear B-spline. Not fancy, but it works
	for (int i = 1; i < totalControlPoints; i++)
		controlPoints[i] = fireBase + (float)i * controlPointStep * fireTop;

	// Standard knot spacing
	for (int i = 0; i < numOfKnots; i++)
	{
		UV[i] = (float)i / (float)numOfKnots; // give thy Y value for the UV coordinates

		if (i < degree)
			knots[i] = 0.f;
		else if (i > m + 1)
			knots[i] = 1.f;
		else
			knots[i] = knots[i-1] + (1.f / (numOfKnots - (2.f * order)));
	}

	// generate the final knot points
	for (int knot = 0; knot < totalControlPoints; knot++)
		for (int i = 0; i < numOfKnots; i++)
			points[knot] += controlPoints[i] * spline(knots[knot], knots, numOfKnots, i, degree);

	// need this otherwise last points will be 0
	for (int i = numOfKnots - degree - 1; i < numOfKnots; i++)
		C[i] = controlPoints[totalControlPoints - 1];

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

	glGenBuffers(1, &fireUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UV), UV, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);


	bool redo = false;
	for (int i = 0; i < numOfKnots; i++)
		if (isnan(C[i].x) || isnan(C[i].y) || isnan(C[i].z))
		{
			redo = true;
			std::cout << "nan" << std::endl;
			break;
		}
	if (redo) createFireVertexBuffer();
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
