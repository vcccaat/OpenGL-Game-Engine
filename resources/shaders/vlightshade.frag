#version 330

uniform sampler2D ipos;
uniform sampler2D inorm;
uniform sampler2D idiff;

in vec2 geom_texCoord;

out vec4 color;

void main() {
	vec4 pos = texture(ipos, geom_texCoord);
	vec4 norm = texture(inorm, geom_texCoord);
	vec4 diff = texture(idiff, geom_texCoord);
	color = norm; // change this to change color
}