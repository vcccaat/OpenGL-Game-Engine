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
	
	float phi, r;
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
	/*vec3 randPt = vec3(0, 0, 0.1);
	vec3 globalPt = eyeSpacePos.xyz + mN * randPt ; 
	vec4 screenPt = mP * vec4(globalPt, 1);
	vec3 outPt = (screenPt.xyz / screenPt.w) * .5 + .5;
	float sampleDepth = texture(idepth, outPt.xy).r;
	if(sampleDepth < outPt.z - .00001) {
		color = vec4(0, 0, 0, 1);
	} else {
		color = vec4(diff * power * pi, rawdiff.a);
	}
	*/
	

	// Testing for occlusion - 3.1.4
	// First part: generate random points
	vec3 randPts[numPts];
	// These seeds are different for each fragment
	vec2 seed1 = vec2(pos.x, pos.y);
	vec2 seed2 = vec2(pos.z, pos.x);
	for(int i = 0; i < numPts; i++) {
		// Changes seeds based on iteration, does not multiply by 0
		float r1 = random(seed1 * (i + 1));
		float r2 = random(seed2 * (i + 1));
		
		randPts[i].xy = squareToUniformDiskConcentric(r1, r2);
		randPts[i].z = sqrt(max(0, 1.0f - pow(randPts[i].x, 2) - pow(randPts[i].y, 2)));
	}

	// Second part: check the fraction of all random points that are occluded
	float occlusion = 0;
	for(int i = 0; i < numPts; i++) {
		vec3 randPt = randPts[i];
		vec3 globalPt = eyeSpacePos.xyz + mN * randPt;
		vec4 screenPt = mP * vec4(globalPt, 1);
		vec3 outPt = (screenPt.xyz / screenPt.w) * .5 + .5;
		float sampleDepth = texture(idepth, outPt.xy).r;
		if(sampleDepth < outPt.z - .00001) {
			occlusion += 1.0;
			// If larger than multiple, do not count
			// TEMP
			//if(abs(sampleDepth - outPt.z) > 4 * range) {
			//	occlusion -= 1.0;
			//}
		}
		
	}
	occlusion /= numPts;
	color = vec4(diff * power * pi * (1 - occlusion), rawdiff.a);
}