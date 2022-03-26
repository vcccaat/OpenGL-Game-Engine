#version 330

uniform sampler2D ipos;
uniform sampler2D inorm;
uniform sampler2D idiff;

in vec2 geom_texCoord;

out vec4 color;

void main() {
	vec4 rawpos = texture(ipos, geom_texCoord);
	vec4 rawnorm = texture(inorm, geom_texCoord);
	vec4 rawdiff = texture(idiff, geom_texCoord);

	vec3 pos = rawpos.yzw;
	vec3 norm = rawnorm.yzw;
	vec3 diff = rawdiff.xyz;
	float alpha = rawpos.x;
	float eta = rawnorm.x;

	// Added constants
	vec3 lightDir = vec3(1, 1, 1);
	float k_a = 0;

	vec3 cnorm = (gl_FrontFacing) ? norm : -norm;
    float NdotH = max(dot(normalize(cnorm), normalize(lightDir)), 0.0);
    color = vec4(k_a + NdotH * diff, 1.0);
}