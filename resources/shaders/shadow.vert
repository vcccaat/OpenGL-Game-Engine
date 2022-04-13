#version 330

uniform mat4 mVlight;  // light View matrix
uniform mat4 mPlight;  // light Projection matrix
uniform mat4 mM;  // position model matrix

layout (location = 0) in vec3 position;


void main() {
    // vertex position in light space
    vec3 vPosition = ( mVlight * mM * vec4(position, 1.0)).xyz;
    gl_Position = mPlight * vec4(vPosition, 1.0);
}