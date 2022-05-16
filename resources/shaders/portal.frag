/*
 * Written for Cornell CS 5625 (Interactive Computer Graphics).
 * Copyright (c) 2015, Department of Computer Science, Cornell University.
 * 
 * This code repository has been authored collectively by:
 * Ivaylo Boyadzhiev (iib2), John DeCorato (jd537), Asher Dunn (ad488), 
 * Pramook Khungurn (pk395), Sean Ryan (ser99), and Eston Schweickart (ers273)
 */

#version 140

uniform sampler2D image;

in vec2 geom_texCoord;
in vec4 screenSpace;

out vec4 fragColor;

void main() {		
	vec4 color = texture(image, geom_texCoord);

	fragColor = color;
 }
