#version 330

in vec3 vPosition;
in vec3 vNormal;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;

uniform float alpha;
uniform float eta;
uniform vec3 diffuseReflectance;

void main() {
	gPosition = vec4(vPosition, 1);
	gNormal = vec4(vNormal / 2 + .5, eta/10);
	gDiffuse = vec4(diffuseReflectance, alpha/10);
}