#version 430 core

layout(lines) in;
layout(triangle_Strip, max_vertices = 146) out;

uniform mat4 modelview;
uniform mat4 projection;

in vec3 vert[];
in vec3 vel[];
in float UV[];

out vec3 vertex;
out vec3 UVS;

const float PI = 3.14159265359,
			scale = 0.3f,
			numOfSlices = 8.f, // if update, change max verticies. so = 18 * numOSlices
			err = 0.0001f;


//Rodrigues' rotation formula
vec3 rotAny (vec3 vector, vec3 axis, float angle) 
{
	return vec3((vector * cos(angle)) + 
				(cross(axis, vector) * sin(angle)) + 
				(axis * dot(axis, vector) * (1.f - cos(angle))));
}


void main (void)
{
	// http://http.developer.nvidia.com/GPUGems/gpugems_ch39.html
	// https://stackoverflow.com/questions/23472048/projecting-3d-points-to-2d-plane
	
	vec3 	cube[8],
			uvs[8];
	int count = 0;

	// for each of the 2 vertices that enter this program (lines provide 2 verticies per input), find the 8 corners of the fire volume they define
	for (int i = 0; i < 2; i++)
	{
		vertex = vert[i];
		vec4 perp, perpRot;

		if (vel[i].z != 0.f)
		{
			perp = vec4(1.f, 0.f, vel[i].x / vel[i].z, 0.f);
			perpRot = vec4(rotAny(perp.xyz, normalize(vel[i]), PI / 2.f), 0.f);
		}
		else if (vel[i].y != 0)
		{
			perp = vec4(1.f, vel[i].x / vel[i].y, 0.f, 0.f);
			perpRot = vec4(rotAny(perp.xyz, normalize(vel[i]), PI / 2.f), 0.f);
		}
		else 
		{
			perp = vec4(1.f, 0.f, 0.f, 0.f);
			perpRot = vec4(0.f, 0.f, 1.f, 0.f);
		}
		
		perp = normalize(perp) * scale;
		perpRot = normalize(perpRot) * scale;
		
		cube[count] = (modelview * (gl_in[i].gl_Position + perpRot)).xyz;
		uvs[count] = vec3(0.f, UV[i], 1.f);
		count++;
		cube[count] = (modelview * (gl_in[i].gl_Position + perp)).xyz; 	
		uvs[count] = vec3(1.f, UV[i], 0.f);	
		count++;
		cube[count] = (modelview * (gl_in[i].gl_Position - perpRot)).xyz; 
		uvs[count] = vec3(0.f, UV[i], -1.f);	
		count++;
		cube[count] = (modelview * (gl_in[i].gl_Position - perp)).xyz;	
		uvs[count] = vec3(-1.f, UV[i], 0.f);	
		count++;
	}
	
	int		minI = 0, 
			maxI = 0;
	float	minZ = cube[0].z, 
			maxZ = cube[0].z;
	// cube[0] is auto assigned so it is starting min by default
	for (int i = 1; i < 8; i++)
	{
		if (cube[i].z < minZ){minZ = cube[i].z; minI = i;}
		if (cube[i].z > maxZ){maxZ = cube[i].z; maxI = i;}
	}

	// indicies in cube array that define all edges in the cube
	ivec2 edges[12] = 
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
	
	// plane normal points from camera to center of cube
	vec3 planeNormal = cube[0];
	for (int i = 1; i < 8; i++)
		planeNormal += cube[i];
	planeNormal = normalize(planeNormal / 8.f);
	
	
	for (	
		vec3 planePoint = cube[maxI];
		planePoint.z > minZ;  
		planePoint += planeNormal * ((maxZ - minZ) / numOfSlices) // normal points from camera to cube so add since we are walking from min distance to max distance
		)
	{
		vec3 	points[6],	// list of points intersecting plane. max intersections = 6
				interprededUVs[6];
		int c = 0; 	// counts entries in points. used for indexing
		// check each edge for intersection wiht the plane
		for (int edge = 0; edge < edges.length(); edge++)
		{	
			// convert the edge into a vector
			vec3 origin = cube[edges[edge].x];
			vec3 direction = normalize(cube[edges[edge].x] - cube[edges[edge].y]);
			
			
			float denominator = dot(direction, planeNormal);
			float numerator = dot(planePoint - origin, planeNormal);
			if (denominator < err && denominator > -err && 
				numerator < err && numerator > -err) // true if edge is parallel and contained in plane
			{
				// isolated, produced 0 results with err of 0.000001f
				// first significant results at err = 0.01
				points[c] = cube[edges[edge].x];
				c++;
				points[c] = cube[edges[edge].y];
				c++;
			}
			else if (denominator > err || denominator < -err)
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
				if (angle1 < err && angle1 > -err)		// first point is on the plane
				{
					// isolated, produced 0 results with error of 0.000001f
					// first significant results produced at err = 0.1
					// some results at 0.01
					points[c] = cube[edges[edge].x];
					c++;
				}
				else if (angle2 < err && angle2 > -err)	// second point is on the plane, else if b/c both cannot be on the plane
				{
					// isolated, produced 0 results with error of 0.000001f
					// some results at 0.01
					// significant results 0.1
					points[c] = cube[edges[edge].y];
					c++;
				}
				else if ((angle1 > err && angle2 < -err) ||	// point 1 = front, point 2 = back
						 (angle1 < -err && angle2 > err))	// point 1 = back, point 2 = front	
				{
					// produces all results with error of 0.000001f, no errors
					// start losing triangles at 0.01
					points[c] = origin + ((numerator / denominator) * direction);
					c++;
				}
			}// else no intersection
		}
		// at this point, points[] contains all of the intersection points between the plane and the edges

		vec3	sortedPoints[6],
				sortedUVS[6];


		for (int i = 0; i < c; i++)
		{
			float minAngle = atan(points[i].x, points[i].y);
			int minIndex = i;
			for (int j = i + 1; j < c; j++)
			{
				float angle = atan(points[j].x, points[j].y);
				if (angle < minAngle)
				{
					minAngle = angle;
					minIndex = j;
				}
			}
			sortedPoints[i] = points[minIndex];
			vec3 tempCoor = points[i];
			points[i] = points[minIndex];
			points[minIndex] = tempCoor;
		}
		
		
		
		// the points are now sorted by angle from plane x axis
		
		
		if (c < 3) continue;	// if there are less than 3 intersection points, cannot form a triangle. stop
		else if (c == 3)	// if there are 3 points, just a trianlge and be done with it.
		{
			gl_Position = projection * vec4(sortedPoints[0], 1.f);
			UVS = sortedUVS[0];
			vertex = gl_Position.xyz;	
			EmitVertex();
			
			gl_Position = projection * vec4(sortedPoints[1], 1.f); 
			UVS = sortedUVS[1];
			vertex = gl_Position.xyz;	
			EmitVertex();
			
			gl_Position = projection * vec4(sortedPoints[2], 1.f);
			UVS = sortedUVS[2];
			vertex = gl_Position.xyz; 	
			EmitVertex();
			
			EndPrimitive();
		}else{
			// create as many triangles as we need to fill the plane.
			// needs to loop so final is % c
			for (int i = 0; i < c; i++)
			{
				gl_Position = projection * vec4(sortedPoints[i], 1.f);
				UVS = sortedUVS[i];
				vertex = gl_Position.xyz;			
				EmitVertex();
				
				gl_Position = projection * vec4(sortedPoints[(i + 1) % c], 1.f);
				UVS = sortedUVS[i];
				vertex = gl_Position.xyz;	
				EmitVertex();
				 	
				gl_Position = projection * vec4(planePoint, 1.f);
				UVS = sortedUVS[i];
				vertex = vec3(0.f, 1.f, 0.f);
				EmitVertex();
				
				EndPrimitive();
			}
		}
	}
}













