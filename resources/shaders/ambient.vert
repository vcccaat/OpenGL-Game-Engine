#version 330

layout (location = 0) in vec3 vert_position;
layout (location = 1) in vec2 vert_texCoord;

out vec2 geom_texCoord;

void main() {
	gl_Position = vec4(vert_position, 1.0);
	geom_texCoord = vert_texCoord;
}