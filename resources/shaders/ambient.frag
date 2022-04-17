#version 330

uniform sampler2D inorm;
uniform sampler2D idiff;
uniform sampler2D idepth;

uniform mat4 mL;
uniform mat4 mV;
uniform mat4 mP;

uniform vec3 power;
uniform vec3 lightPos;
uniform float range;

in vec2 geom_texCoord;

out vec4 color;

const float pi = 3.14159265358979323846264;
const int numPts = 64;

float random(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 squareToUniformDiskConcentric(float r1, float r2) {
	r1 = 2.0 * r1 - 1.0;
	r2 = 2.0 * r2 - 1.0;
	
	float phi;
	float r;
    if (r1 == 0 && r2 == 0) {
        r = 0;
		phi = 0;
    } else if (r1*r1 > r2*r2) {
        r = r1;
        phi = (pi/4.0f) * (r2/r1);
    } else {
        r = r2;
        phi = (pi/2.0f) - (r1/r2) * (pi/4.0f);
    }
	
    return vec2(r * cos(phi), r * sin(phi));
	
	//float a = r1 * r1 + r2 * r2;
	//float b = sqrt(a);
	//float c = atan(r2, r1);
	//return vec2(b, c);
}

void main() {
	// Take relevant data
	vec4 rawnorm = texture(inorm, geom_texCoord);
	vec4 rawdiff = texture(idiff, geom_texCoord);
    float rawDepth = texture(idepth, geom_texCoord).r;
	vec4 pos = vec4(geom_texCoord.x*2.0-1.0, geom_texCoord.y*2.0-1.0, rawDepth*2.0-1.0,1.0); 
	vec3 norm = normalize((rawnorm.xyz -.5) * 2);
	vec3 diff = rawdiff.xyz;
	float alpha = rawdiff.w * 10;
	float eta = rawnorm.w * 10;
	
	// Compute eyeSpacePos
	vec4 viewSpacePos = inverse(mP) * pos;
    vec3 eyeSpacePos = (viewSpacePos.xyz / viewSpacePos.w).xyz;
    vec4 worldSpacePos = inverse(mV) * vec4(eyeSpacePos, 1);

	// Compute tangents
	float smallest = min(min(norm.x, norm.y), norm.z);
	vec3 notParallel;
	if(smallest == norm.x) notParallel = vec3(1, 0, 0);
	else if(smallest == norm.y) notParallel = vec3(0, 1, 0);
	else notParallel = vec3(0, 0, 1);
	vec3 tang1 = normalize(cross(norm, notParallel));
	vec3 tang2 = normalize(cross(norm, tang1));
	mat3 mN = mat3(tang1, tang2, norm);

	// Testing for occlusion - 3.1.3
	/*
	vec3 randPt = vec3(0, 0, 0.1);
	vec3 globalPt = worldSpacePos.xyz + mN * randPt; 
	vec4 screenPt = mP * mV * vec4(globalPt, 1);
	vec3 outPt = (screenPt.xyz / screenPt.w) * .5 + .5;
	float sampleDepth = texture(idepth, outPt.xy).r;
	if(sampleDepth < outPt.z - .00001) {
		color = vec4(0, 0, 0, 1);
	} else {
		color = vec4(diff * power * pi, rawdiff.a);
	}
	//color = vec4(normalize(eyeSpacePos - globalPt), 1);
	//color = vec4(norm, 1);
	//color = viewSpacePos;
	//color = screenPt;
	*/
	

	// Testing for occlusion - 3.1.4
	// First part: generate random points
	
	vec3 randPts[numPts];
	// These seeds are different for each fragment
	vec2 seed1 = vec2(pos.x, pos.y * pos.z);
	vec2 seed3 = vec2(pos.y, pos.x * pos.z);
	vec2 seed2 = vec2(pos.z, pos.x * pos.y);
	for(int i = 0; i < numPts; i++) {
		// Changes seeds based on iteration, does not multiply by 0
		float r1 = random(seed1 * (i + 1));
		float r2 = random(seed2 * (i + 1));
		float r3 = random(seed3 * (i + 1));
		
		randPts[i].xy = squareToUniformDiskConcentric(r1, r2);
		randPts[i].z = sqrt(1.0f - randPts[i].x);
		randPts[i] *= (range * r3);
	}

	// Second part: check the fraction of all random points that are occluded
	float occlusion = 0;
	for(int i = 0; i < numPts; i++) {
		vec3 randPt = randPts[i];
		vec3 globalPt = worldSpacePos.xyz + mN * randPt;
		vec4 screenPt = mP * mV * vec4(globalPt, 1);
		vec3 outPt = (screenPt.xyz / screenPt.w) * .5 + .5;
		float sampleDepth = texture(idepth, outPt.xy).r * 2 - 1;
		vec4 a = inverse(mP) * vec4(outPt.xy, sampleDepth, 1);
		vec3 b = (a.xyz / a.w).xyz;
		vec3 c = (inverse(mV) * vec4(b, 1)).xyz;	
		// If larger than multiple, do not count
		if(length(c) < length(outPt) - .00001 && distance(c, outPt) < 4 * range) {
			occlusion += 1.0;
		}
		
	}
	occlusion /= numPts;
	float scale = .1; // decreasing contribution to 10% to improve visual look
	color = vec4(diff * power * pi * (1 - occlusion) * scale, rawdiff.a);
}