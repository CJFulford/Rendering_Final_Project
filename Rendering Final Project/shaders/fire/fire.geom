#version 430 core

layout(lines) in;
layout(triangle_Strip, max_vertices = 90) out;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec3 cam;

in vec3 vert[];
in vec3 norm[];
in vec3 vel[];

out vec3 vertex;

const float PI = 3.14159265359,
			scale = 0.25f,
			numOfSlices = 5f, // if update, change max verticies. so = 18 * numOSlices
			err = 0.0001f;


//Rodrigues' rotation formula
vec3 rotAny (vec3 vector, vec3 axis, float angle) 
{
	return vec3((vector * cos(angle)) + 
				(cross(axis, vector) * sin(angle)) + 
				(axis * dot(axis, vector) * (1f - cos(angle))));
}

float distance(vec3 v1, vec3 v2)
{
	return sqrt(((v1.x - v2.x) * (v1.x - v2.x)) +
				((v1.y - v2.y) * (v1.y - v2.y)) +
				((v1.z - v2.z) * (v1.z - v2.z)));
}


void main (void)
{
	bool test = false;
	//test = !test;
	
	if (test)
	{	
		for (int i = 0; i < 2; i++)
		{
			vertex = vert[i];

			vec4 perp, perpRot;

			if (vel[i].z > err)
			{
				perp = vec4(normalize(vec3(1f, 0f, -vel[i].x / vel[i].z)), 0f) * scale;
				perpRot = vec4(normalize(rotAny(perp.xyz, normalize(vel[i]), PI/2f)), 0f) * scale;
			}
			else if (vel[i].y > err)
			{
				perp = vec4(normalize(vec3(1f, -vel[i].x / vel[i].y, 0f)), 0f) * scale;
				perpRot = vec4(normalize(rotAny(perp.xyz, normalize(vel[i]), PI/2f)), 0f) * scale;
			}
			else 
			{
				perp = vec4(normalize(vec3(0f, 0f, 1f)), 0f) * scale;
				perpRot = vec4(normalize(vec3(1f, 0f, 0f)), 0f) * scale;
			}

			// doing this gives fixed positions for the corners of the volumes
			/*
			gl_Position = projection * modelview * (gl_in[i].gl_Position + perpRot);
			EmitVertex();
			gl_Position = projection * modelview * (gl_in[i].gl_Position + perp); 
			EmitVertex();
			gl_Position = projection * modelview * (gl_in[i].gl_Position - perpRot); 
			EmitVertex();
			gl_Position = projection * modelview * (gl_in[i].gl_Position - perp);	
			EmitVertex();
			*/
			// this makes everything disappear. I have no idea where they are
			// i have turned off control restrictions and still could not find them
			/*
			gl_Position = modelview * (gl_in[i].gl_Position + perpRot);
			EmitVertex();
			gl_Position = modelview * (gl_in[i].gl_Position + perp); 
			EmitVertex();
			gl_Position = modelview * (gl_in[i].gl_Position - perpRot); 
			EmitVertex();
			gl_Position = modelview * (gl_in[i].gl_Position - perp);	
			EmitVertex();
			*/
			// this gives the points in a fixe position at the front of the screen
			gl_Position = (gl_in[i].gl_Position + perpRot);
			EmitVertex();
			gl_Position = (gl_in[i].gl_Position + perp); 
			EmitVertex();
			gl_Position = (gl_in[i].gl_Position - perpRot); 
			EmitVertex();
			gl_Position = (gl_in[i].gl_Position - perp);	
			EmitVertex();
		}
		EndPrimitive();
	}
	else
	{	
		vec3 cube[8];
		int count = 0;

		// for each of the 2 vertices that enter this program (lines provide 2 verticies per input), find the 8 corners of the fire volume they define
		for (int i = 0; i < 2; i++)
		{
			vertex = vert[i];

			vec4 perp, perpRot;

			if (vel[i].z > err)
			{
				perp = vec4(normalize(vec3(1f, 0f, -vel[i].x / vel[i].z)), 0f) * scale;
				perpRot = vec4(normalize(rotAny(perp.xyz, normalize(vel[i]), PI/2f)), 0f) * scale;
			}
			else if (vel[i].y > err)
			{
				perp = vec4(normalize(vec3(1f, -vel[i].x / vel[i].y, 0f)), 0f) * scale;
				perpRot = vec4(normalize(rotAny(perp.xyz, normalize(vel[i]), PI/2f)), 0f) * scale;
			}
			else 
			{
				perp = vec4(normalize(vec3(0f, 0f, 1f)), 0f) * scale;
				perpRot = vec4(normalize(vec3(1f, 0f, 0f)), 0f) * scale;
			}
									// this one does not change based on camera location. do not use
									/*
									cube[count] = (gl_in[i].gl_Position + perpRot).xyz;
									count++;
									cube[count] = (gl_in[i].gl_Position + perp).xyz; 		
									count++;
									cube[count] = (gl_in[i].gl_Position - perpRot).xyz; 	
									count++;
									cube[count] = (gl_in[i].gl_Position - perp).xyz;		
									count++;
									*/
			// this one allows changes based on camera location
			// after model view tansform, min z is farthest from camera
			cube[count] = (modelview * (gl_in[i].gl_Position + perpRot)).xyz;
			count++;
			cube[count] = (modelview * (gl_in[i].gl_Position + perp)).xyz; 		
			count++;
			cube[count] = (modelview * (gl_in[i].gl_Position - perpRot)).xyz; 	
			count++;
			cube[count] = (modelview * (gl_in[i].gl_Position - perp)).xyz;		
			count++;
			/*
			cube[count] = (projection * modelview * (gl_in[i].gl_Position + perpRot)).xyz;
			count++;
			cube[count] = (projection * modelview * (gl_in[i].gl_Position + perp)).xyz; 		
			count++;
			cube[count] = (projection * modelview * (gl_in[i].gl_Position - perpRot)).xyz; 	
			count++;
			cube[count] = (projection * modelview * (gl_in[i].gl_Position - perp)).xyz;		
			count++;
			*/
		}

		// take the stock camera coordinates and transform them for movement.
		// so slices stay view dependent
		//vec3 cam = (projection * modelview * vec4(cam, 1f)).xyz;
		//vec3 cam = (modelview * vec4(cam, 1f)).xyz;
		//vec3 cam = vec3(0f, 0f, 0f);

		
		/*
		int		minI = 0, maxI = 0;
		float	minD = distance(cam, cube[0]), maxD = minD;
		// cube[0] is auto assigned so it does not need to be checked
		for (int i = 1; i < 8; i++)
		{
			float d = distance(cam, cube[i]);
			if (d < minD){minD = d; minI = i;}
			if (d > maxD){maxD = d; maxI = i;}
		}
		*/
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
		ivec2 indicies[] = 
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
		
		// list for recording frontface vs back face for each vertex. is cube so 8 vertices
		int front[8];
		
		// http://http.developer.nvidia.com/GPUGems/gpugems_ch39.html
		
		//vec3 planeNormal = normalize(cam - cube[minI]);
		vec3 planeNormal = normalize(cube[maxI]);

		// for how ever many slices per volume, walk the slicing plane forward, calculate the vertices, and send out the triangles
		/*
		for (	
			vec3 planePoint = cube[minI];
			distance(planePoint, cam) < maxD;  
			planePoint += planeNormal * ((maxD - minD) / numOfSlices) // normal points from camera to cube so add since we are walking from min distance to max distance
			)
		*/
		for (	
			vec3 planePoint = cube[maxI];
			planePoint.z > minZ;  
			planePoint +=/*points into neg so add*/ planeNormal * ((maxZ - minZ) / numOfSlices) // normal points from camera to cube so add since we are walking from min distance to max distance
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
			for (int i = 0; i < indicies.length(); i++)
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
			vec3 midPoint = vec3(0f, 0f, 0f);
			for (int i = 0; i < c; i++)
				midPoint += points[i];
			midPoint / c;
			
			//Sort the polygon vertices clockwise or counterclockwise by projecting them onto the x-y plane and computing their angle around the center, with the first vertex or the x axis as the reference. Note that to avoid trigonometric computations, the tangent of the angle and the sign of the coordinates, combined into a single scalar value called the pseudo-angle, can be used for sorting the vertices (Moret and Shapiro 1991).
			
			// plane is defined by planePoint and planeNormal
			// x and y axis on plane (to make it 2d) ar defined as x = points[0] as an arbitrary x direction and y = points[0] rotated 90 degrees abouy plane normal. both x and y directions need to be normalized
			// https://stackoverflow.com/questions/23472048/projecting-3d-points-to-2d-plane
			
			vec3 planeX = normalize(points[0]);
			vec3 planeY = normalize(rotAny(planeX, planeNormal, PI / 2f));
			
			
			// start at 1 because angle of points[0] will be 0 degrees obviously
			float angles[6];
			angles[0] = 0f;
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
			// the points are now sorted by angle from plane x axis
			
			// if there are 3 points, just ma a trianlge and be done with it.
			if (c == 3)
			{
				gl_Position = projection * modelview * vec4(sortedPoints[0], 1f);	EmitVertex();
				gl_Position = projection * modelview * vec4(sortedPoints[1], 1f); 	EmitVertex();
				gl_Position = projection * modelview * vec4(sortedPoints[2], 1f); 	EmitVertex();
				EndPrimitive();
				continue;
			}
			
			// create as many triangles as we need to fill the plane.
			// needs to loop so final is % c
			for (int i = 0; i < c; i++)
			{
				gl_Position = projection * modelview * vec4(sortedPoints[i], 1f);			EmitVertex();
				gl_Position = projection * modelview * vec4(sortedPoints[(i + 1) % c], 1f); 	EmitVertex();
				gl_Position = projection * modelview * vec4(midPoint, 1f); 					EmitVertex();
			}
			EndPrimitive();
		}
	}
}













