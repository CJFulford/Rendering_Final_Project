#version 430 core

out vec4 color;

uniform sampler2D image;
uniform float time;
uniform float radius = 0.2f;

in vec3 uvFrag;
const float PI = 3.14159265359;

// ==========================================================
// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 c){
    return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p, float freq ){
    float unit = 500.f/freq; //float unit = screenWidth/freq;
    vec2 ij = floor(p/unit);
    vec2 xy = mod(p,unit)/unit;
    //xy = 3.*xy*xy-2.*xy*xy*xy;
    xy = .5*(1.-cos(PI*xy));
    float a = rand((ij+vec2(0.,0.)));
    float b = rand((ij+vec2(1.,0.)));
    float c = rand((ij+vec2(0.,1.)));
    float d = rand((ij+vec2(1.,1.)));
    float x1 = mix(a, b, xy.x);
    float x2 = mix(c, d, xy.x);
    return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res){
    float persistance = .5;
    float n = 0.;
    float normK = 0.;
    float f = 1.;
    float amp = 1.;
    int iCount = 0;
    for (int i = 0; i<50; i++){
        n+=amp*noise(p, f);
        f*=2.;
        normK+=amp;
        amp*=persistance;
        if (iCount == res) break;
        iCount++;
    }
    float nf = n/normK;
    return nf*nf*nf*nf;
}
// ==========================================================






void main (void)
{
	vec3 UV = uvFrag;
	float t = UV.z + (time * 400.f);
	vec2 uv;
	 
	
	float  	octaves 	= 3.f,
			gain 		= 0.5,	
			lacunarity 	= 1.f / gain,	// inverse gain for peturbance
			turb		= 0.f,			// records the total peturbance
			length 		= 1.f,			// fire height, base to top
			frequency 	= 1.f,			// application specific
			offset 		= 1.f, 			// number of fires
			stabFact	= 1.f;			// stability factor
		
		
	vec4 noiseScale = vec4(radius, radius, length, 1.f) * frequency;
	vec4 noisePosition = vec4(UV.x, UV.y, UV.z - t, t);
		
	// scale the position of the noise with the position of the fire
	for (int i = 0; i < 4; i++)
		noisePosition[i] *= noiseScale[i];
		
	for (float i = 0.f; i < octaves; i++)
		turb += pow(gain, i) * pNoise(noisePosition.zw, 50);

	float u = sqrt((UV.x * UV.x) + (UV.y * UV.y));	
	
	float v = (turb + 1.f) / 2.f;
	color = vec4(texture(image, vec2(u, v)).xyz, 1.f / 8.f);	// dont know why but the UV in the book appear to be backwards
}















