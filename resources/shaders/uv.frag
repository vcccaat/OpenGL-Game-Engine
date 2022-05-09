#version 330

in vec2 uv0;

in vec3 vPosition;
in vec3 vNormal;

uniform sampler2D diffuseTexture;

out vec4 fragColor;


void main() {
  
  fragColor = texture(diffuseTexture,uv0);
}