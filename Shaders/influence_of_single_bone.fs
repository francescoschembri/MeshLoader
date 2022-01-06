#version 330 core
out vec4 FragColor;

in vec4 aColor;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;


void main()
{    
    FragColor = aColor;
}

