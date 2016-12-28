#version 430 core

layout(lines) in;
layout(triangle_strip, max_vertices = 90) out;

uniform mat4 modelview;
uniform mat4 projection;

in vec3 vert[];
in vec3 norm[];
in vec3 vel[];

out vec3 vertex;

const float PI = 3.14159265359,
			scale = 0.25f,
			numOfSlices = 5.f, // if update, change max verticies. so = 18 * numOSlices
			err = 0.0001f;


//Rodrigues' rotation formula
vec3 rotAny (vec3 vector, vec3 axis, float angle) 
{
	return vec3((vector * cos(angle)) + 
				(cross(axis, vector) * sin(angle)) + 
				(axis * dot(axis, vector) * (1.f - cos(angle))));
}

float distance(vec3 v1, vec3 v2)
{
	return sqrt(((v1.x - v2.x) * (v1.x - v2.x)) +
				((v1.y - v2.y) * (v1.y - v2.y)) +
				((v1.z - v2.z) * (v1.z - v2.z)));
}


void main (void)
{
	vec3 cube[8];
	int count = 0;

	// for each of the 2 vertices that enter this program (lines provide 2 verticies per input), find the 8 corners of the fire volume they define
	for (int i = 0; i < gl_in.length(); i++)
	{
		vertex = vert[i];

		vec4 perp, perpRot;

		if (vel[i].z > err)
		{
			perp = vec4(normalize(vec3(1.f, 0.f, -vel[i].x / vel[i].z)), 0.f) * scale;
			perpRot = vec4(normalize(rotAny(perp.xyz, normalize(vel[i]), PI/2.f)), 0.f) * scale;
		}
		else if (vel[i].y > err)
		{
			perp = vec4(normalize(vec3(1.f, -vel[i].x / vel[i].y, 0.f)), 0.f) * scale;
			perpRot = vec4(normalize(rotAny(perp.xyz, normalize(vel[i]), PI/2.f)), 0.f) * scale;
		}
		else 
		{
			perp = vec4(normalize(vec3(0.f, 0.f, 1.f)), 0.f) * scale;
			perpRot = vec4(normalize(vec3(1.f, 0.f, 0.f)), 0.f) * scale;
		}

		cube[count] = (modelview * (gl_in[i].gl_Position + perpRot)).xyz; 	count++;
		cube[count] = (modelview * (gl_in[i].gl_Position + perp)).xyz; 		count++;
		cube[count] = (modelview * (gl_in[i].gl_Position - perpRot)).xyz; 	count++;
		cube[count] = (modelview * (gl_in[i].gl_Position - perp)).xyz;		count++;
	}

	// take the stock camera coordinates and transform them for movement.
	// so slices stay view dependent
	//vec3 cam = (modelview * vec4(vec3(0.f, 0.3f, 2.f), 0.f)).xyz;
	vec3 cam = vec3(0.f, 0.3f, 2.f);

	
	int		minI = 0, maxI = 0;
	float	minD = distance(cam, cube[0]), maxD = minD;
	for (int i = 1; i < 8; i++)
	{
		float d = distance(cam, cube[i]);
		if (d < minD){minD = d; minI = i;}
		if (d > maxD){maxD = d; maxI = i;}
	}

	// indicies in cube array that define all edges in volume
	ivec2 indicies[] = 
	{	ivec2(0,1),
		ivec2(0,3),
		ivec2(0,4),
		ivec2(1,2),
		ivec2(1,5),
		ivec2(2,3),
		ivec2(2,6),
		ivec2(3,7),
		ivec2(4,5),
		ivec2(4,7),
		ivec2(5,6),
		ivec2(6,7)};
	
	// list for recording frontface vs back face for each vertex. is cube so 8 vertices
	int front[8];
	
	// http://http.developer.nvidia.com/GPUGems/gpugems_ch39.html
	
	vec3 planeNormal = normalize(cam - cube[minI]);

	// for how ever many slices per volume, walk the slicing plane forward, calculate the vertices, and send out the triangles
	for (	
		vec3 planePoint = cube[minI]; 
		distance(planePoint, cam) < maxD;  
		planePoint += planeNormal * ((maxD - minD) / numOfSlices) // normal points from camera to cube so add since we are walking from min distance to max distance
		)
	{
		// check if each vertex is front facing or back facing
		for (int i = 0; i < 8; i++)
		{
			float d = dot(planePoint - cube[i], planeNormal);
			if (d > err) // pos dot product = front facing
				front[i] = 0;
			else if (d < -err) // neg dot product = back facing
				front[i] = 1;
			else // dot = 0 point is on plane
				front[i] = 2;
		}
		

		vec3 points[6];	// list of points intersecting plane. max intersections = 6
		int c = 0;	// to count intersection points
		
		//check plane against all 12 edges for intersection
		for (int i = 0; i < 12; i++)
		{
			// check each point. if front=2, point is on plane. add to points if 2 and continue.
			bool toggle = false;
			if (front[indicies[i].x] == 2)
			{
				points[c] = cube[indicies[i].x];
				c++;
				toggle = true;
			}
			if (front[indicies[i].y] == 2)
			{
				points[c] = cube[indicies[i].y];
				c++;
				toggle = true;
			}
			if (toggle) continue;

			// if this point is reached, neither point is on the plane
			// the plane will intersect an edge only if one vertex is in front and the other is behind (one of the vertices is 1 and the other is 0.)
			if (front[indicies[i].x] != front[indicies[i].y])
			{
				// treat the edge as a ray and find intersection with the plane
				// this is one of the coordinates of the slices
				vec3 dir = normalize(cube[indicies[i].x] - cube[indicies[i].y]);
				vec3 origin = cube[indicies[i].x];

				// ray-plane intersection
				float d = dot(dir, planeNormal);
				// this should always be true. otherwise, the direction is orthoganol to the plane normal and will not cause an intersection.
				// so it will either have been caught be front = 2 opn both points if the points ly on the plane or 
				// they are both front facing or both back facing
				if (d > err || d < -err)		
				{
					points[c] = origin + ((dot((planePoint - origin), planeNormal) / d) * dir);
					c++;
				}
			}
		}
		// at this point, points[] contains all of the intersection points between the plane and the volume
		
		// if i cannot make a triangle
		if (c < 3) continue;
		
		// always sort for normal consistancy
		
		// find center point by averaging the intersection points
		vec3 midPoint = vec3(0.f, 0.f, 0.f);
		for (int i = 0; i < c; i++)
			midPoint += points[i];
		midPoint / c;
		
		//Sort the polygon vertices clockwise or counterclockwise by projecting them onto the x-y plane and computing their angle around the center, with the first vertex or the x axis as the reference. Note that to avoid trigonometric computations, the tangent of the angle and the sign of the coordinates, combined into a single scalar value called the pseudo-angle, can be used for sorting the vertices (Moret and Shapiro 1991).
		
		// plane is defined by planePoint and planeNormal
		// x and y axis on plane (to make it 2d) ar defined as x = points[0] as an arbitrary x direction and y = points[0] rotated 90 degrees abouy plane normal. both x and y directions need to be normalized
		// https://stackoverflow.com/questions/23472048/projecting-3d-points-to-2d-plane
		
		vec3 planeX = normalize(points[0]);
		vec3 planeY = normalize(rotAny(planeX, planeNormal, PI / 2.f));
		
		
		// start at 1 because angle of points[0] will be 0 degrees obviously
		float angles[6];
		for (int i = 1; i < c; i++)
			angles[i] = atan(dot(planeX, points[i] - midPoint), dot(planeY, points[i] - midPoint));
		
		vec3 sortedPoints[6];
		sortedPoints[0] = points[0];
		
		// again start from 1 because points[0] angle will be smallest
		for (int i = 1; i < c; i++)
		{
			float minA = angles[i];
			int minI = i;
			for (int j = i + 1; j < c; j++)
				if (angles[j] < angles[i])
				{
					minA = angles[j];
					minI = j;
				}
			sortedPoints[i] = points[minI];
		}
		// the points are now sorted counterclockwise
		
		// create as many triangles as we need to fill the plane.
		// needs to loop so final is % c
		/*if (c == 3)
		{
			gl_Position = projection * vec4(sortedPoints[0], 0.f);	EmitVertex();
			gl_Position = projection * vec4(sortedPoints[1], 0.f); 	EmitVertex();
			gl_Position = projection * vec4(sortedPoints[2], 0.f); 	EmitVertex();
			EndPrimitive();
			continue;
		}*/
		for (int i = 0; i < c; i++)
		{
			gl_Position = projection * modelview * vec4(0.1f, 0.1f, 0.0f, 0.f); EmitVertex();
			gl_Position = projection * modelview * vec4(-0.1f, 0.1f, 0.0f, 0.f); EmitVertex();
			gl_Position = projection * modelview * vec4(0.0f, 0.2f, 0.0f, 0.f); EmitVertex();
			//gl_Position = projection * vec4(sortedPoints[i], 0.f);			EmitVertex();
			//gl_Position = projection * vec4(sortedPoints[(i + 1) % c], 0.f); 	EmitVertex();
			//gl_Position = projection * vec4(midPoint, 0.f); 					EmitVertex();
		}
		EndPrimitive();
	}
}













