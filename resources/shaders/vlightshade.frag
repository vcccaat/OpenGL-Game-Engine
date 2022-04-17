#version 330

uniform sampler2D inorm;
uniform sampler2D idiff;
uniform sampler2D idepth;
uniform sampler2D ishadowmap;


uniform mat4 mVlight;
uniform mat4 mPlight;
uniform mat4 mV;  // View matrix
uniform mat4 mL;  // Light matrix
uniform mat4 mP;  // projection matrix

uniform vec3 power;
uniform vec3 lightPos;

in vec2 geom_texCoord;


out vec4 color;

const float PI = 3.14159265358979323846264;

// The Fresnel reflection factor
float fresnel(vec3 i, vec3 m, float eta) {
  float c = abs(dot(i,m));
  float g = sqrt(eta*eta - 1.0 + c*c);

  float gmc = g-c;
  float gpc = g+c;
  float nom = c*(g+c)-1.0;
  float denom = c*(g-c)+1.0;
  return 0.5*gmc*gmc/gpc/gpc*(1.0 + nom*nom/denom/denom);
}

// The one-sided Smith shadowing/masking function
float G1(vec3 v, vec3 m, vec3 n, float alpha) {
  float vm = dot(v,m);
  float vn = dot(v,n);
  if (vm*vn > 0.0) {
    float cosThetaV = dot(n,v);
    float sinThetaV2 = 1.0 - cosThetaV*cosThetaV;
    float tanThetaV2 = sinThetaV2 / cosThetaV / cosThetaV;
    return 2.0 / (1.0 + sqrt(1.0 + alpha*alpha*tanThetaV2));
  } else {
    return 0;
  }
}

// The GGX slope distribution function
float D(vec3 m, vec3 n, float alpha) {
  float mn = dot(m,n);
  if (mn > 0.0) {
    float cosThetaM = mn;
    float cosThetaM2 = cosThetaM*cosThetaM;
    float tanThetaM2 = (1.0 - cosThetaM2) / cosThetaM2;
    float cosThetaM4 =  cosThetaM*cosThetaM*cosThetaM*cosThetaM;
    float X = (alpha*alpha + tanThetaM2);
    return alpha*alpha / (PI * cosThetaM4 * X * X);
  } else {
    return 0.0;
  }
}

// Evalutate the Microfacet BRDF (GGX variant)
float isotropicMicrofacet(vec3 i, vec3 o, vec3 n, float eta, float alpha) {

    float odotn = dot(o,n);
    vec3 m = normalize(i + o);

    float idotn = dot(i,n);
    if (idotn <= 0.0 || odotn <= 0.0)
        return 0;

    float idotm = dot(i,m);
    float F = (idotm > 0.0) ? fresnel(i,m,eta) : 0.0;
    float G = G1(i,m,n,alpha) * G1(o,m,n,alpha);
    return F * G * D(m,n,alpha) / (4.0*idotn*odotn);
}


void main() {
	vec4 rawnorm = texture(inorm, geom_texCoord);
	vec4 rawdiff = texture(idiff, geom_texCoord);
    float rawDepth = texture(idepth, geom_texCoord).r;

	vec4 pos = vec4(geom_texCoord.x*2.0-1.0,geom_texCoord.y*2.0-1.0,rawDepth*2.0-1.0,1.0); 
	vec3 norm = normalize((rawnorm.xyz -.5) * 2);
	vec3 diff = rawdiff.xyz;
	float alpha = rawdiff.w * 10;
	float eta = rawnorm.w * 10;

	vec3 vLightPos = (mV *  mL * vec4(lightPos, 1.0)).xyz;  
	vec4 viewSpacePos = inverse(mP) * pos;
    vec3 eyeSpacePos = (viewSpacePos.xyz / viewSpacePos.w).xyz;

	vec3 vCamPos = vec3(0,0,0);
	vec3 wo = normalize(vLightPos - eyeSpacePos);  
	vec3 wi = normalize(vCamPos - eyeSpacePos);
	float Kspecular = isotropicMicrofacet(wi, wo, norm, eta, alpha);  
	float NdotH = max(dot(norm, wo), 0.0);

	float divise = NdotH / (4 * PI  * pow(length(vLightPos - eyeSpacePos), 2)); 
	

  // shadow 
  vec4 worldSpacePos = inverse(mV) * vec4(eyeSpacePos,1.0);
  vec4 lightSpacePos =  mPlight * mVlight * worldSpacePos;
  vec3 light_texCoord =  (lightSpacePos.xyz/lightSpacePos.w) / 2 + 0.5; 
  float closestDepth = texture(ishadowmap, light_texCoord.xy).r;
  float currentDepth = light_texCoord.z;

  if(closestDepth < currentDepth - 0.0001){
    color = vec4(0,0,0,1);
  }
  else {
    color = vec4(Kspecular * power + diff * power * 1/PI, 1.0) * divise;
  }

  //  color = vec4(lightSpacePos); 
  
}