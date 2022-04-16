#version 330

//precision highp float;

const int nBlur = 5;
const float ks[nBlur] = float[nBlur](.8843, .1, .012, .0027, .001);

uniform sampler2D b0;
uniform sampler2D b1;
uniform sampler2D b2;
uniform sampler2D b3;
uniform sampler2D b4;

in vec2 geom_texCoord;

out vec4 fragColor;

void main() {
	// Get each texture
	vec4 blurs[nBlur] = vec4[nBlur]( texture(b0, geom_texCoord), texture(b1, geom_texCoord), texture(b2, geom_texCoord), texture(b3, geom_texCoord), texture(b4, geom_texCoord) );

	// Sum the textures
	vec4 color = vec4(0, 0, 0, 0);
	for(int i = 0; i < nBlur; i++) {
		color = color + ks[i] * blurs[i];
	}
	color.a = 1;
	
	//vec4 color0 = texture(b0, geom_texCoord);
	//vec4 color1 = texture(b1, geom_texCoord);
	//vec4 color2 = texture(b2, geom_texCoord);
	//vec4 color3 = texture(b3, geom_texCoord);
	//vec4 color4 = texture(b4, geom_texCoord);
	fragColor = color;
}
