#version 430 core

out vec4 color;

uniform sampler2D image;
uniform float time;

in vec3 vertex, uvFrag;
in float radius;



float turb(vec4 position, int octaves)
{
	float gain = 0.5;
	float lacunarity = 2;
	float turb = 0.f;
	
	for (float i = 0.f; i < octaves; i++)
		turb += pow(gain, i) * noise4(position * pow(lacunarity, i));
	return turb;
}

// a stability factor times the square root of the height leads to visually pleasing results
float kernel(float input)
{
	float stabilityFactor = 1.f;
	return stabilityFactor * sqrt(input);;
}



void main (void)
{
	
	bool perlin = true;
	//perlin = !perlin;
	
	
	vec2 uv;
	float length = 1.f;		// fire height, base to top
	float frequency = 1.f;	// application specific
	float offset = 1.f; 	// number of fires 
	
	if (perlin)
	{
		int octaves = 3;
		
		vec4 noiseScale = vec4(radius, radius, length, 1.f) * frequency;
		vec4 noisePosition = vec4(uvFrag.xy, uvFrag.z - time, time);
		
		for (int i = 0; i < 4; i++)
			noisePosition[i] *= noiseScale[i];
		
		float u = sqrt((uvFrag.x * uvFrag.x) + (uvFrag.y * uvFrag.y));
		float v = uvFrag.z + kernel(uvFrag.z) * turb(noisePosition + offset, octaves);
		color = vec4(texture(image, vec2(v,u)).xyz, 1.f / 2.f);	// dont know why but the UV in the book appear to be backwards
	}	
	
	
	
	
	
	
	else // basic stuff
	{
		uv.y = sqrt((uvFrag.x * uvFrag.x) + (uvFrag.y * uvFrag.y));
		uv.x = uvFrag.z;
		
		vec3 col = texture(image, uv).xyz;

		float limit = 0.05f;
		if (col.x < limit && col.y <limit && col.z < limit) color = vec4(0.f, 0.f, 0.f, 0.f);
		else color = vec4(col, 1.f / 2.f);
	}
}















