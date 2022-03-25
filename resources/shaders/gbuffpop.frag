#version 330

layout (location = 0) out vec4 gSpecular;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;

in vec3 vPosition;
in vec3 vNormal;

uniform float alpha;
uniform float eta;
uniform vec3 diffuseReflectance;

void main() {
	gSpecular = vec4(alpha,diffuseReflectance.yz, eta);
	gNormal = vec4(vNormal/2+vec3(0.8), 1);
	gDiffuse = vec4(diffuseReflectance, 1);
}