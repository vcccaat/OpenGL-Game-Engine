#version 330

uniform mat4 mV;  // light View matrix
uniform mat4 mP;  // light Projection matrix
uniform mat4 mM;  // position model matrix

layout (location = 0) in vec3 position;


void main() {
    // vertex position in light space
    vec3 vPosition = ( mV * mM * vec4(position, 1.0)).xyz;
    gl_Position = mP * vec4(vPosition, 1.0);
}