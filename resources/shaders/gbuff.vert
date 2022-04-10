/*
 * Written for Cornell CS 5625 (Interactive Computer Graphics).
 * Copyright (c) 2015, Department of Computer Science, Cornell University.
 * 
 * This code repository has been authored collectively by:
 * Ivaylo Boyadzhiev (iib2), John DeCorato (jd537), Asher Dunn (ad488), 
 * Pramook Khungurn (pk395), Sean Ryan (ser99), and Eston Schweickart (ers273)
 */

// Simple full screen quad
#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 vPosition;  // vertex position in eye space
out vec3 vNormal; // vertex normal in eye space

uniform mat4 mM;  // Model matrix
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Projection matrix

void main() {
    vPosition = (mV * mM * vec4(position, 1.0)).xyz;
    gl_Position = mP * vec4(vPosition, 1.0);
    vNormal = ( mM * vec4(normal, .0)).xyz;
}
