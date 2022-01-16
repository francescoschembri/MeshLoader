#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform vec3 light_pos;

void main()
{    
    FragColor = texture(texture_diffuse, TexCoords);
}

