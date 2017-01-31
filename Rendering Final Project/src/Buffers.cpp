#include "SceneShader.h"
#include <iostream>
#include <vector>

//float spline(float knot, float knots[], int numOfKnots, int i, int p)
float spline(std::vector<float> knots, int i, int k)
{
	if (k == 1 && knots[i] <= i && i <= knots[i + 1])
		return 1;
	else if (k == 1)
		return 0;
	else
	{
		float ratio1 = 0.f, ratio2 = 0.f;
		// if denominator of a term = 0, leave the whole term as 0
		if (abs(knots[i + k - 1] - knots[i]) > err)
			ratio1 = (i - knots[i]) / (knots[i + k - 1] - knots[i]);
		if (abs(knots[i + k] - knots[i + 1]) > err)
			ratio2 = (knots[i + k] - i) / (knots[i + k] - knots[i + 1]);

		return	(ratio1 * spline(knots, i, k - 1)) + 
				(ratio2 * spline(knots, i + 1, k - 1));
	}
}

void SceneShader::createFireVertexBuffer()
{
	const int	totalControlPoints = 10,
                m = totalControlPoints - 1,
                degree = 3,
                order = degree + 1,
                numOfKnots = m + order;

	float controlPointStep = 1.f / (m);

	std::vector<float> knots, UV;
	std::vector<glm::vec3> controlPoints, points, velocity;
	
	vec3	fireBase(0.f, 0.f, 0.f), fireTop(0.f, 1.f, 0.f);


	// generate linear set of control points. Not fancy, but it works
	for (int i = 0; i < totalControlPoints; i++)
		controlPoints.push_back(fireBase + (float)i * controlPointStep * fireTop);

	// Standard knot spacing
	for (int i = 0; i < numOfKnots; i++)
	{
		// give the Y value for the UV coordinates
		UV.push_back((float)i / (float)numOfKnots); 

		if (i < degree)
			knots.push_back(0.f);
		else if (i > m + 1)
			knots.push_back(1.f);
		else
			knots.push_back(knots[i-1] + (1.f / (numOfKnots - (2.f * order))));
	}

	// generate the curve points from the knot points
	for (int controlPoint = 0; controlPoint < totalControlPoints; controlPoint++)
	{
		glm::vec3 point(0.f, 0.f, 0.f);
		for (int i = 0; i < numOfKnots; i++)
			point += controlPoints[controlPoint] * spline(knots, i, order);
		points.push_back(point);
	}

	// need this otherwise last points will be 0
//	for (int i = numOfKnots - degree - 1; i < numOfKnots; i++)
	//	points[i] = controlPoints[totalControlPoints - 1];

	for (int i = 0; i < numOfKnots; i++)
	{
		if (i == numOfKnots - 1)
			velocity.push_back(points[i - 1] - points[i]);
		else
			velocity.push_back(points[i] - points[i + 1]);
	}

	printVecVector(points);

	fireGeneratedPoints = sizeof(points) / sizeof(points[0]);

	fireTexture = loadTexture("textures/fire_profile_texture.png");

	glGenVertexArrays(1, &fireVertexArray);
	glBindVertexArray(fireVertexArray);

	glGenBuffers(1, &fireVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &fireVelocityBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireVelocityBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(velocity), &velocity, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &fireUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fireUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UV), &UV, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

    
	bool redo = false;
	for (int i = 0; i < numOfKnots; i++)
		if (isnan(points[i].x) || isnan(points[i].y) || isnan(points[i].z))
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
