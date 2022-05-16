#version 330

in vec2 uv0;

in vec3 vPosition;
in vec3 vNormal;

uniform sampler2D diffuseTexture;

uniform float alpha;
uniform float eta;
uniform vec3 diffuseReflectance;
uniform vec3 camPos;
uniform vec3 lightPos;
uniform mat4 mV;// View matrix
uniform mat4 mC;// Camera matrix
uniform vec3 power;
uniform int textureMapped;

in vec4 screenPos;

const float amb=.75;

out vec4 fragColor;

// This is a shader code fragment (not a complete shader) that contains
// the functions to evaluate the microfacet BRDF.

const float PI=3.14159265358979323846264;

// The Fresnel reflection factor
//   i -- incoming direction
//   m -- microsurface normal
//   eta -- refractive index
float fresnel(vec3 i,vec3 m,float eta){
  float c=abs(dot(i,m));
  float g=sqrt(eta*eta-1.+c*c);
  
  float gmc=g-c;
  float gpc=g+c;
  float nom=c*(g+c)-1.;
  float denom=c*(g-c)+1.;
  return.5*gmc*gmc/gpc/gpc*(1.+nom*nom/denom/denom);
}

// The one-sided Smith shadowing/masking function
//   v -- in or out vector
//   m -- microsurface normal
//   n -- (macro) surface normal
//   alpha -- surface roughness
float G1(vec3 v,vec3 m,vec3 n,float alpha){
  float vm=dot(v,m);
  float vn=dot(v,n);
  if(vm*vn>0.){
    float cosThetaV=dot(n,v);
    float sinThetaV2=1.-cosThetaV*cosThetaV;
    float tanThetaV2=sinThetaV2/cosThetaV/cosThetaV;
    return 2./(1.+sqrt(1.+alpha*alpha*tanThetaV2));
  }else{
    return 0.;
  }
}

// The GGX slope distribution function
//   m -- microsurface normal
//   n -- (macro) surface normal
//   alpha -- surface roughness
float D(vec3 m,vec3 n,float alpha){
  float mn=dot(m,n);
  if(mn>0.){
    float cosThetaM=mn;
    float cosThetaM2=cosThetaM*cosThetaM;
    float tanThetaM2=(1.-cosThetaM2)/cosThetaM2;
    float cosThetaM4=cosThetaM*cosThetaM*cosThetaM*cosThetaM;
    float X=(alpha*alpha+tanThetaM2);
    return alpha*alpha/(PI*cosThetaM4*X*X);
  }else{
    return 0.;
  }
}

// Evalutate the Microfacet BRDF (GGX variant) for the paramters:
//   i -- incoming direction (unit vector, pointing away from surface)
//   o -- outgoing direction (unit vector, pointing away from surface)
//   n -- outward pointing surface normal vector
//   eta -- refractive index
//   alpha -- surface roughness
// return: scalar BRDF value
float isotropicMicrofacet(vec3 i,vec3 o,vec3 n,float eta,float alpha){
  
  float odotn=dot(o,n);
  vec3 m=normalize(i+o);
  
  float idotn=dot(i,n);
  if(idotn<=0.||odotn<=0.)
  return 0.;
  
  float idotm=dot(i,m);
  float F=(idotm>0.)?fresnel(i,m,eta):0.;
  float G=G1(i,m,n,alpha)*G1(o,m,n,alpha);
  return F*G*D(m,n,alpha)/(4.*idotn*odotn);
}

vec4 shade(vec3 lightPos,vec3 power){
  
  vec3 normal=(gl_FrontFacing)?normalize(vNormal):normalize(-vNormal);
  
  // in eye space
  vec3 vLightPos=(mV*vec4(lightPos,1.)).xyz;
  vec3 vCamPos=(mV*mC*vec4(camPos,1.)).xyz;
  vec3 wo=normalize(vLightPos-vPosition);
  vec3 wi=normalize(vCamPos-vPosition);
  float Kspecular=isotropicMicrofacet(wi,wo,normal,eta,alpha);
  float NdotH=max(dot(normal,wo),0.);
  
  float divise=NdotH/(4*PI*pow(length(vLightPos-vPosition),2));
  vec4 color=vec4(Kspecular*power+diffuseReflectance*1/PI*power,1.)*divise;
  
  //return vec4(normal, 1); //TEMP//
  return color;
}

void main(){
  vec4 acc=vec4(0,0,0,0);
  for(int i=0;i<1;i++){
    // vec3 lightPos = pointLightPositions[i];
    // vec3 power =  pointLightPower[i];
    acc=acc+shade(lightPos,power);
    //break; //TEMP//
  }
  acc=acc+vec4(diffuseReflectance*vec3(amb,amb,amb),0);
  if(textureMapped==0.){
    fragColor=acc;
  }
  else if(textureMapped==2){
    //PORTALS
    vec2 screenSpace = uv0; // Fix Pls
    fragColor=texture(diffuseTexture,uv0);
  }
  else{
    fragColor=texture(diffuseTexture,uv0);
  }
  
}
