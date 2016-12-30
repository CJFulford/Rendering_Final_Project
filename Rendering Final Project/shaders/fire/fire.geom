#version 430 core

layout(lines) in;
layout(triangle_Strip, max_vertices = 144) out;

uniform mat4 	modelview, projection;
in vec3  vel[];
in float UVGeomIn[];
out vec3 uvFrag;
out float radius;

const float PI = 3.14159265359,
			scale = 0.2f,
			numOfSlices = 8.f, // if update, change max verticies. so = 18 * numOSlices
			err = 0.00001f;


//Rodrigues' rotation formula
vec3 rotAny (vec3 vector, vec3 axis, float angle) 
{
	return vec3((vector * cos(angle)) + 
				(cross(axis, vector) * sin(angle)) + 
				(axis * dot(axis, vector) * (1.f - cos(angle))));
}


void main (void)
{
	
	radius = scale;
	// Main Article: http://graphics.cs.ucdavis.edu/~hamann/FullerKrishnanMahrousHamannJoyFirePaperFor_I3D2007AsSubmitted11012006.pdf
	// http://http.developer.nvidia.com/GPUGems/gpugems_ch39.html
	
	vec3 	cube[8], uvGeom[8];
	int count = 0;

	// for each of the 2 vertices that enter this program (lines provide 2 verticies per input), find the 8 corners of the fire volume (cube) they define
	// uvGeom is in here so to coordinate the link between teh corner positions an the texture coordinates
	for (int i = 0; i < gl_in.length(); i++)
	{
		vec4 perp, perpRot;

		if (vel[i].z > err || vel[i].z < -err)
		{
			perp = vec4(1.f, 0.f, vel[i].x / vel[i].z, 0.f);
			perpRot = vec4(rotAny(perp.xyz, normalize(vel[i]), PI / 2.f), 0.f);
		}
		else if (vel[i].y > err || vel[i].y < -err)
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
		uvGeom[count] = vec3(1.f, UVGeomIn[i], -1.f);
		count++;
		
		cube[count] = (modelview * (gl_in[i].gl_Position + perp)).xyz; 	
		uvGeom[count] = vec3(-1.f, UVGeomIn[i], 1.f);	
		count++;
		
		cube[count] = (modelview * (gl_in[i].gl_Position - perpRot)).xyz; 
		uvGeom[count] = vec3(1.f, UVGeomIn[i], -1.f);	
		count++;
		
		cube[count] = (modelview * (gl_in[i].gl_Position - perp)).xyz;	
		uvGeom[count] = vec3(-1.f, UVGeomIn[i], 1.f);	
		count++;
	}
	
	int		maxI = 0;
	float	minZ = cube[0].z, 
			maxZ = cube[0].z;
	// cube[0] is auto assigned so it is starting min by default
	for (int i = 1; i < 8; i++)
	{
		if 		(cube[i].z < minZ){minZ = cube[i].z;}
		else if (cube[i].z > maxZ){maxZ = cube[i].z; maxI = i;}
	}

	// indicies in cube array that define all edges in the cube
	const ivec2 edges[12] = 
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
	planeNormal = normalize(planeNormal / 8.f); // 8 corners for cube
	
	
	for (	
		vec3 planePoint = cube[maxI];
		planePoint.z > minZ;  
		planePoint += planeNormal * ((maxZ - minZ) / (numOfSlices * 2.f)) // normal points from camera to cube so add since we are walking from min distance to max distance
		)
	{
		// list of points intersecting plane. max intersections = 6, need uvGeom to stay linked to coordinate
		vec3 points[6],	pointsUV[6];
		int c = 0; 	// counts entries in points. used for indexing
		// check each edge for intersection wiht the plane
		for (int edge = 0; edge < edges.length(); edge++)
		{	
			// convert the edge into a vector
			vec3 	origin = cube[edges[edge].x],
					direction = normalize(cube[edges[edge].x] - cube[edges[edge].y]);
			
			
			float 	denominator = dot(direction, planeNormal),
					numerator = dot(planePoint - origin, planeNormal);
			if (denominator < err && denominator > -err && numerator < err && numerator > -err) // true if edge is parallel and contained in plane
			{
				points[c] = cube[edges[edge].x];
				pointsUV[c] = uvGeom[edges[edge].x];
				c++;
				
				points[c] = cube[edges[edge].y];
				pointsUV[c] = uvGeom[edges[edge].y];
				c++;
			}
			else if (denominator < -err || denominator > err)
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
					points[c] = cube[edges[edge].x];
					pointsUV[c] = uvGeom[edges[edge].x];
					c++;
				}
				else if (angle2 < err && angle2 > -err)	// second point is on the plane, else if b/c both cannot be on the plane
				{
					points[c] = cube[edges[edge].y];
					pointsUV[c] = uvGeom[edges[edge].y];
					c++;
				}
				else if ((angle1 > err && angle2 < -err) ||	// point 1 = front, point 2 = back
						 (angle1 < -err && angle2 > err))	// point 1 = back, point 2 = front	
				{
					float scalar = numerator / denominator;
					points[c] = origin + (scalar * direction);
					pointsUV[c] = uvGeom[edges[edge].x] + (scalar * normalize(uvGeom[edges[edge].x] - uvGeom[edges[edge].y])); // interpolate UV coord
					c++;
				}
			}// else no intersection
		}
		// at this point, points[] contains all of the intersection points between the plane and the edges
		
		// find center point by averaging the intersection points
		// find the centerUV too
		vec3 midPoint = points[0], midpointUV = uvGeom[0];
		for (int i = 1; i < c; i++)
		{
			midPoint += points[i];
			midpointUV += pointsUV[i];
		}
		midPoint /= c;
		midpointUV /= c;
		

		// sort the points by angle, since they are all already on the same plane, we can just sort by x and y, not z b/c all shoud have the same z relative to camera
		vec3 sortedPoints[6], sortedUVGeom[6], pos[6], neg[6], posUV[6], negUV[6];
		int p = 0, n = 0;
		for (int i = 0; i < c; i++)
		{
			if (points[i].y >= midPoint.y)
			{
				pos[p] = points[i];
				posUV[p] = pointsUV[i];
				p++;
			}
			else
			{
				neg[n] = points[i];
				negUV[n] = pointsUV[i];
				n++;
			}
		}
		
		// sort the points above midpoint from right to left (max x to min x)
		for (int i = 0; i < p; i++)
		{
			int max = i;
			for (int j = i + 1; j < p; j++)
				if (pos[j].x > pos[max].x) max = j;
			
			vec3 temp;
			
			sortedPoints[i] = pos[max];
			temp = pos[i];
			pos[i] = pos[max];
			pos[max] = temp;
			
			sortedUVGeom[i] = posUV[max];
			temp = posUV[i];
			posUV[i] = posUV[max];
			posUV[max] = temp;
		}
		
		// sort the points below midpoint fromt the left to right (minx to max x)
		for (int i = 0; i < n; i++)
		{
			int min = i;
			for (int j = i + 1; j < n; j++)
				if (neg[j].x < neg[min].x) min = j;
			
			vec3 temp;
			
			sortedPoints[i + p] = neg[min];
			temp = neg[i];
			neg[i] = neg[min];
			neg[min] = temp;
			
			sortedUVGeom[i + p] = negUV[min];
			temp = negUV[i];
			negUV[i] = negUV[min];
			negUV[min] = temp;
		}
		
		
		
		
		// the points are now sorted by angle from plane x axis
		
		
		if (c < 3) continue;	// if there are less than 3 intersection points, cannot form a triangle. stop
		else if (c == 3)	// if there are 3 points, just a trianlge and be done with it.
		{
			gl_Position = projection * vec4(sortedPoints[0], 1.f);
			uvFrag = sortedUVGeom[0];	
			EmitVertex();
			
			gl_Position = projection * vec4(sortedPoints[1], 1.f); 
			uvFrag = sortedUVGeom[1];	
			EmitVertex();
			
			gl_Position = projection * vec4(sortedPoints[2], 1.f);
			uvFrag = sortedUVGeom[2]; 	
			EmitVertex();
			
			EndPrimitive();
		}else{
			// create as many triangles as we need to fill the plane.
			// needs to loop so final is % c
			for (int i = 0; i < c; i++)
			{
				gl_Position = projection * vec4(sortedPoints[i], 1.f);
				uvFrag = sortedUVGeom[i];			
				EmitVertex();
				
				gl_Position = projection * vec4(sortedPoints[(i + 1) % c], 1.f);
				uvFrag = sortedUVGeom[(i + 1) % c];
				EmitVertex();
				 	
				gl_Position = projection * vec4(midPoint, 1.f);
				uvFrag = midpointUV;
				EmitVertex();
				
			}
			EndPrimitive();
		}
	}
}













