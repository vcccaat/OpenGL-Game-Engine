#version 330

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gSpecular;

in vec3 vPosition;
in vec3 vNormal;

uniform float alpha;
uniform float eta;
uniform vec3 diffuseReflectance;

void main() {
	gPosition = vec4(vPosition, alpha);
	gNormal = vec4(vNormal, eta);
	gSpecular = vec4(diffuseReflectance, 1);
}