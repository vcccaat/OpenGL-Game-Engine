#version 330

uniform sampler2D ipos;
uniform sampler2D inorm;
uniform sampler2D idiff;

uniform mat4 mV;  // View matrix
//uniform mat4 mL;  // Light matrix
//uniform vec3 lightPos;
//uniform vec3 power;

in vec2 geom_texCoord;

out vec4 color;

void main() {
	vec4 rawpos = texture(ipos, geom_texCoord);
	vec4 rawnorm = texture(inorm, geom_texCoord);
	vec4 rawdiff = texture(idiff, geom_texCoord);

	vec3 pos = rawpos.xyz;
	vec3 norm = rawnorm.xyz;
	vec3 diff = rawdiff.xyz;
	float alpha = rawpos.w;
	float eta = rawnorm.w;

	// Added constants
	//vec3 lightDir = (mV * vec4(1, 1, 1, 1)).xyz;
	vec3 lightDir = vec3(1, 1, 1);
	float k_a = .1;

	vec3 cnorm = (gl_FrontFacing) ? normalize(norm) : -normalize(norm);
    float NdotH = max(dot(cnorm, normalize(lightDir)), 0.0);
    color = vec4(k_a + NdotH * diff, 1.0);
}