#version 330


uniform mat4 mM;  // Model matrix
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Projection matrix

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tex_coord0;
layout (location = 3) in vec3 tex_coord1;

out vec3 vPosition;  // vertex position in eye space
out vec3 vNormal; // vertex normal in eye space

out vec2 uv0;
out vec2 uv1;

void main() {
    vPosition = (mV * mM * vec4(position, 1.0)).xyz;
    gl_Position = mP * vec4(vPosition, 1.0);
    uv0 = tex_coord0.xy;
    uv1 = tex_coord1.xy;

    vNormal = normalize(mV * mM * vec4(normal, .0)).xyz;
}
