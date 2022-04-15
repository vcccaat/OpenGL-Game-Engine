#version 330

const float PI = 3.14159265358979323846264;

uniform vec3 A, B, C, D, E, zenith;
uniform float thetaSun = 60 * PI/180, phiSun = PI;
const float sunAngularRadius = 0.5 * PI/180;
const vec3 solarDiscRadiance = vec3(10000);
const vec3 groundRadiance = vec3(0.5);
const float skyScale = 0.06;

uniform sampler2D inorm;
uniform sampler2D idiff;
uniform sampler2D idepth;

uniform mat4 mV;
uniform mat4 mP;

in vec2 geom_texCoord;

out vec4 color;

vec3 perez(float theta, float gamma) {
    return (1 + A * exp(B / cos(theta))) * (1 + C * exp(D * gamma) + E * pow(cos(gamma), 2.0));
}

vec3 sunRadiance(vec3 dir) {
    vec3 sunDir = vec3(sin(thetaSun) * cos(phiSun), cos(thetaSun), sin(thetaSun) * sin(phiSun));
    return dot(dir, sunDir) > cos(sunAngularRadius) ? solarDiscRadiance : vec3(0);
}

const mat3 XYZ2RGB = mat3(
   3.2404542, -0.969266 ,  0.0556434,
  -1.5371385,  1.8760108, -0.2040259,
  -0.4985314,  0.041556 ,  1.0572252
);

vec3 skyRadiance(vec3 dir) {
    vec3 sunDir = vec3(sin(thetaSun) * cos(phiSun), cos(thetaSun), sin(thetaSun) * sin(phiSun));
    float gamma = acos(min(1.0, dot(dir, sunDir)));
    if (dir.y > 0) {
        float theta = acos(dir.y);
        vec3 Yxy = zenith * perez(theta, gamma) / perez(0, thetaSun);
        return skyScale * XYZ2RGB * vec3(Yxy[1] * (Yxy[0]/Yxy[2]), Yxy[0], (1 - Yxy[1] - Yxy[2])*(Yxy[0]/Yxy[2]));
    } else {
        return groundRadiance;
    }
}

vec3 sunskyRadiance(vec3 dir) {
    return sunRadiance(dir) + skyRadiance(dir);
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
	
    // If not background, assign no color
    if(rawdiff.a != 0) {
	    color = vec4(0, .5, 0, 1);
        return;
	}
	
    // Transform position and camera to world space
	vec4 viewSpacePos = inverse(mP) * pos;
    vec3 eyeSpacePos = (viewSpacePos.xyz / viewSpacePos.w).xyz;
    vec4 worldSpacePos = inverse(mV) * vec4(eyeSpacePos, 1);
	
    vec3 campos = vec3(0,0,0);
	
	vec4 viewSpaceCam = inverse(mP) * vec4(campos, 1);
	vec3 eyeSpaceCam = (viewSpaceCam.xyz / viewSpaceCam.w).xyz;
	vec4 worldSpaceCam = inverse(mV) * vec4(eyeSpaceCam, 1);
	
    // Compute view direction
    vec4 wi = normalize(worldSpaceCam - worldSpacePos);
    color = vec4(sunskyRadiance(wi.xyz), 1);
}