#version 330

uniform mat4 mM;  // Model matrix
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Projection matrix
const int MAX_BONES = 100;
uniform mat4 boneM[MAX_BONES]; // Bone matrices

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in ivec4 boneIds; //init index -1
layout (location = 3) in vec4 boneWts; //init weight 0

out vec3 vPosition;  // vertex position in eye space
out vec3 vNormal; // vertex normal in eye space

const int MAX_BONE_INFLUENCE = 4;

void main() {
    vec4 weightSumPos = vec4(0.0);
    vec4 weightSumNorm = vec4(0.0);

    for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {     
        if (boneIds[i] == -1){
		    if(i == 0) {
			    weightSumPos = vec4(position, 1.0);
                weightSumNorm = vec4(normal, 0.0);
			}
            break;
        }
        weightSumPos += boneM[boneIds[i]] * vec4(position, 1.0) * boneWts[i];
        weightSumNorm += boneM[boneIds[i]] * vec4(normal, 0.0) * boneWts[i];
    }

    // vPosition = (mV * mM * vec4(position, 1.0)).xyz;
    // vNormal = normalize(mV * mM * vec4(normal, 0.0)).xyz;
    vPosition = (mV * mM * weightSumPos).xyz;
    vNormal = normalize(mV * mM * weightSumNorm).xyz;
    gl_Position = mP * vec4(vPosition, 1.0);
}
