#version 330

uniform sampler2D image;

in vec2 geom_texCoord;

out vec4 color;

void main() {
	color = texture(image, geom_texCoord);
}