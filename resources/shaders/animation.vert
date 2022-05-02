#version 330

uniform mat4 mM;  // Model matrix
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Projection matrix
uniform mat4 boneM[4]; // Bone matrices

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in ivec4 boneIds;
layout (location = 3) in vec4 boneWts;

out vec3 vPosition;  // vertex position in eye space
out vec3 vNormal; // vertex normal in eye space

const int MAX_BONE_INFLUENCE = 4;

void main() {
    vec4 weightSumPos = vec4(0.0);
    vec4 weightSumNorm = vec4(0.0);

    for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    
        if(boneIds[i] == -1) {
            continue;
		}
        
        weightSumPos += boneM[i][boneIds[i]] * vec4(position, 1.0) * boneWts[i];
        weightSumNorm += boneM[i][boneIds[i]] * vec4(normal, 0.0) * boneWts[i];
    }

    vPosition = (mV * mM * weightSumPos).xyz;
    gl_Position = mP * vec4(vPosition, 1.0);
    vNormal = normalize(mV * mM * weightSumNorm).xyz;
}
