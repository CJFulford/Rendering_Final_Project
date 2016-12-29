#version 430 core

layout(lines) in;
layout(triangle_Strip, max_vertices = 146) out;

uniform mat4 modelview;
uniform mat4 projection;

in vec3 vert[];
in vec3 vel[];

out vec3 vertex;
out vec3 col;

const float PI = 3.14159265359,
			scale = 0.25f,
			numOfSlices = 8.f, // if update, change max verticies. so = 18 * numOSlices
			err = 0.f;


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
	// http://http.developer.nvidia.com/GPUGems/gpugems_ch39.html
	// https://stackoverflow.com/questions/23472048/projecting-3d-points-to-2d-plane
	
	vec3 cube[8];
	int count = 0;

	// for each of the 2 vertices that enter this program (lines provide 2 verticies per input), find the 8 corners of the fire volume they define
	for (int i = 0; i < 2; i++)
	{
		vertex = vert[i];
		vec4 perp, perpRot;

		if (vel[i].z > err || vel[i].z < -err)
		{
			perp = vec4(normalize(vec3(1.f, 0.f, -vel[i].x / vel[i].z)), 0.f) * scale;
			perpRot = vec4(normalize(rotAny(perp.xyz, normalize(vel[i]), PI / 2.f)), 0.f) * scale;
		}
		else if (vel[i].y > err || vel[i].y < -err)
		{
			perp = vec4(normalize(vec3(1.f, -vel[i].x / vel[i].y, 0.f)), 0.f) * scale;
			perpRot = vec4(normalize(rotAny(perp.xyz, normalize(vel[i]), PI / 2.f)), 0.f) * scale;
		}
		else 
		{
			perp = vec4(normalize(vec3(1.f, 0.f, 0.f)), 0.f) * scale;
			perpRot = vec4(normalize(vec3(0.f, 0.f, 1.f)), 0.f) * scale;
		}
		cube[count] = (modelview * (gl_in[i].gl_Position + perpRot)).xyz;	count++;
		cube[count] = (modelview * (gl_in[i].gl_Position + perp)).xyz; 		count++;
		cube[count] = (modelview * (gl_in[i].gl_Position - perpRot)).xyz; 	count++;
		cube[count] = (modelview * (gl_in[i].gl_Position - perp)).xyz;		count++;
	}
	
	int		minI = 0, 
			maxI = 0;
	float	minZ = cube[0].z, 
			maxZ = cube[0].z;
	// cube[0] is auto assigned so it does not need to be checked
	for (int i = 1; i < 8; i++)
	{
		if (cube[i].z < minZ){minZ = cube[i].z; minI = i;}
		if (cube[i].z > maxZ){maxZ = cube[i].z; maxI = i;}
	}

	// indicies in cube array that define all edges in volume
	ivec2 edges[] = 
	{	
		ivec2(0,1),
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
		ivec2(6,7)
	};
	
	
	vec3 planeNormal = normalize(cube[maxI]);
	for (	
		vec3 planePoint = cube[maxI];
		planePoint.z > minZ;  
		planePoint += planeNormal * ((maxZ - minZ) / numOfSlices) // normal points from camera to cube so add since we are walking from min distance to max distance
		)
	{
		vec3 points[6];	// list of points intersecting plane. max intersections = 6
		int c = 0; 	// counts entries in points. used for indexing
		// check each edge for intersection wiht the plane
		for (int edge = 0; edge < edges.length(); edge++)
		{	
			// convert the edge into a vector
			vec3 origin = cube[edges[edge].x];
			vec3 direction = normalize(cube[edges[edge].x] - cube[edges[edge].y]);
			
			// line plane intersection equation found on wikipedia
			// since both vectors are normalized, there should be no errors
			float denominator = dot(direction, planeNormal);
			float numerator = dot(planePoint - origin, planeNormal);
			if (denominator == 0.f && numerator == 0.f) // true if edge is parallel and contained in plane
			{
				points[c] = cube[edges[edge].x];
				c++;
				points[c] = cube[edges[edge].y];
				c++;
			}
			else	
			{
				// ray will intersect the plane
				// however we are dealing with a non infinite edge.
				// we cannot use the ray formula (P = P_0 + tD) to restrict the ray b/c we cannot divide vectors and we need to to find t
				// we solve this dilema by finding if the points are in front of or behind the plane
				// the edge will only intersect the plane if one of the vertices is in front and the other is behind the plane.
				// the case of both vertices being on the plane is a parallel case which has already been dealt with
				// we can determine if a point is front facing or back facing by computing its dot product with the plane normal
				// a positive dot product means the point is front facing and a negative dot product means the point is back facing.
				// a 0 dot product (-err < x < err becasue of floats) means the point lies on the plane and does intersect it
				
				float angle1 = dot(planePoint - cube[edges[edge].x], planeNormal);
				float angle2 = dot(planePoint - cube[edges[edge].y], planeNormal);
				
				// account for points being on the plane first
				if (angle1 > -err && angle1 < err)		// first point is on the plane
				{
					points[c] = cube[edges[edge].x];
					c++;
				}
				else if (angle2 > -err && angle2 < err)	// second point is on the plane, else if b/c both cannot be on the plane
				{
					points[c] = cube[edges[edge].y];
					c++;
				}
				else									// neither point lies on the plane
				{
					if ((angle1 >  err && angle2 < -err) ||	// point 1 = front, point 2 = back
						(angle1 < -err && angle2 >  err))	// point 1 = back, point 2 = front	
					{
						float scalar = numerator / denominator;
						points[c] = origin + (scalar * direction);
						c++;
					}
					// else no intersection
				}
			}
		}
		// at this point, points[] contains all of the intersection points between the plane and the volume

		
		
		
		
		// find center point by averaging the intersection points
		vec3 midPoint = points[0];
		for (int i = 1; i < c; i++)
			midPoint += points[i];
		
		float ct = c;	// for division by a float
		midPoint / ct;
		
		
		
		
		
		
		// find 2 points on the plane that aare perpendicular to the plane normal and to each other to form2 basis vectors
		vec3 planeX, planeY;
		if (planeNormal.z > err || planeNormal.z < -err)
		{
			planeX = normalize(vec3(1.f, 0.f, -planeNormal.x / planeNormal.z));
			planeY = normalize(rotAny(planeX.xyz, normalize(planeNormal), PI / 2.f));
		}
		else if (planeNormal.y > err || planeNormal.y < -err)
		{
			planeX = normalize(vec3(1.f, -planeNormal.x / planeNormal.y, 0.f));
			planeY = normalize(rotAny(planeX.xyz, normalize(planeNormal), PI / 2.f));
		}
		else 
		{
			planeX = normalize(vec3(0.f, 0.f, 1.f));
			planeY = normalize(vec3(1.f, 0.f, 0.f));
		}
		
		
		
		
		
		
		// project the intersection points onto the plane defined by planeX and planeY with normal of planeNormal and origin of midpoint
		vec2 planeCoords[6];
		for (int i = 0; i < c; i++)
			planeCoords[i] = vec2(	dot(planeX, midPoint - points[i]),
									dot(planeY, midPoint - points[i]));
		
		
		
		// sort the points by angle relative to the midpoint and the coordinate bases defined by planeX and planeY
		vec3 sortedPoints[6];		
		for (int i = 0; i < c; i++)
		{
			float minAngle = atan(planeCoords[i].y, planeCoords[i].x);
			int minIndex = i;
			for (int j = i + 1; j < c; j++)
			{
				float angle = atan(planeCoords[j].y, planeCoords[j].x);
				if (angle < minAngle)
				{
					minAngle = angle;
					minIndex = j;
				}
			}
			sortedPoints[i] = points[minIndex];
			vec2 tempCoor = planeCoords[i];
			planeCoords[i] = planeCoords[minIndex];
			planeCoords[minIndex] = tempCoor;
		}
		
		
		
		
		
		
		// the points are now sorted by angle from plane x axis
		
		
		if (c < 3) continue;	// if there are less than 3 intersection points, cannot form a triangle. stop
		else if (c == 3)	// if there are 3 points, just a trianlge and be done with it.
		{
			col = vec3(0.f, 0.f, 0.f);
			gl_Position = projection * vec4(sortedPoints[0], 1.f);	EmitVertex(); 	col +=  sortedPoints[0];
			gl_Position = projection * vec4(sortedPoints[1], 1.f); 	EmitVertex(); 	col +=  sortedPoints[1];
			gl_Position = projection * vec4(sortedPoints[2], 1.f); 	EmitVertex(); 	col +=  sortedPoints[2];
			col /= 3;
			EndPrimitive();
		}else{
			// create as many triangles as we need to fill the plane.
			// needs to loop so final is % c
			for (int i = 0; i < c; i++)
			{
			col = vec3(0.f, 0.f, 0.f);
				gl_Position = projection * vec4(sortedPoints[i], 1.f);				EmitVertex(); 	col +=  sortedPoints[i];
				gl_Position = projection * vec4(sortedPoints[(i + 1) % c], 1.f); 	EmitVertex(); 	col +=  sortedPoints[(i + 1) % c];
				gl_Position = projection * vec4(midPoint, 1.f); 					EmitVertex(); 	col +=  midPoint;
				col /= 3;
				EndPrimitive();
			}
		}
	}
}













