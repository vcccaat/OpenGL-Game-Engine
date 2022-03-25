#version 330 core
out vec4 FragColor;

in vec2 geom_texCoord;

uniform sampler2D image;

void main()
{             
    FragColor = texture(image, geom_texCoord);
}
