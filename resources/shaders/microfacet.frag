#version 330

uniform float alpha;
uniform float eta;
uniform vec3 diffuseReflectance;
uniform vec3 camPos;
uniform vec3 lightPos;

//uniform vec3 lightDir;
// uniform vec3 k_d;
// uniform vec3 k_a;

uniform mat4 mV;  // View matrix
uniform mat4 mL;  // Light matrix
uniform vec3 power;

in vec3 vPosition;
in vec3 vNormal;

out vec4 fragColor;

// This is a shader code fragment (not a complete shader) that contains 
// the functions to evaluate the microfacet BRDF.

const float PI = 3.14159265358979323846264;

// The Fresnel reflection factor
//   i -- incoming direction
//   m -- microsurface normal
//   eta -- refractive index
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
//   v -- in or out vector
//   m -- microsurface normal
//   n -- (macro) surface normal
//   alpha -- surface roughness
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
//   m -- microsurface normal
//   n -- (macro) surface normal
//   alpha -- surface roughness
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

// Evalutate the Microfacet BRDF (GGX variant) for the paramters:
//   i -- incoming direction (unit vector, pointing away from surface)
//   o -- outgoing direction (unit vector, pointing away from surface)
//   n -- outward pointing surface normal vector
//   eta -- refractive index
//   alpha -- surface roughness
// return: scalar BRDF value
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
    vec3 normal = (gl_FrontFacing) ? normalize(vNormal) : normalize(-vNormal);

    // Same as from lambert.frag
    //float NdotH = max(dot(normalize(normal), normalize(lightDir)), 0.0);
    //fragColor = vec4(k_a + NdotH * k_d, 1.0);

    // New try
    // vec3 vLightPos = (mV * vec4(lightPos, 1.0)).xyz;
    // vec3 wi = normalize(vLightPos - vPosition);
    // //vec3 vCamPos = (mV * vec4(camPos, 1.0)).xyz;
    // vec3 wo = normalize(camPos - vPosition);
    // float bsdfOut = isotropicMicrofacet(wi, wo, normal, eta, alpha);
    // float NdotH = max(dot(normalize(normal), normalize(wi)), 0.0);
    // //fragColor = vec4(diffuseReflectance + bsdfOut, 1.0); // caused very monochromatic bunny
    // fragColor = vec4(bsdfOut + NdotH * diffuseReflectance, 1.0) / pow(length(vLightPos - vPosition), 2); // a little better. do we need to divide by r^2?
   

    // in world space 
    // vec3 worldPos = (inverse(mV) * vec4(vPosition,1.0)).xyz;
    // vec3 worldNormal = (inverse(mV) * vec4(normal,1.0)).xyz;
    // vec3 vLightPos = (vec4(lightPos, 1.0)).xyz;
    // vec3 wo = normalize(vLightPos - worldPos);
    // vec3 wi = normalize(camPos - worldPos);
    // float Kspecular = 1/PI ;//isotropicMicrofacet(wi, wo, worldNormal, eta, alpha);  
    // float NdotH = max(dot(normal, wi), 0.0);

    // vec3 irradiance = vec3(2,2,2);//power / pow(length(vLightPos - vPosition), 2);

    // fragColor = vec4(irradiance[0]*diffuseReflectance[0],irradiance[1]*diffuseReflectance[1],irradiance[2]*diffuseReflectance[2], 1.0) * NdotH * Kspecular;


    // in eye space   
    vec3 vLightPos = (mV * mL * vec4(lightPos, 1.0)).xyz;
    vec3 vCamPos = (mV * vec4(camPos, 1.0)).xyz;
    vec3 wo = normalize(vLightPos - vPosition);
    vec3 wi = normalize(vCamPos - vPosition);
    float Kspecular = isotropicMicrofacet(wi, wo, normal, eta, alpha);  
    float NdotH = max(dot(normal, wi), 0.0);

    // the power is 1000 not 80 tho
    float divise = NdotH / (4 * PI * pow(length(vLightPos - vPosition), 2));
    fragColor = vec4(Kspecular * power + diffuseReflectance * 1/PI * power, 1.0) * divise;


}