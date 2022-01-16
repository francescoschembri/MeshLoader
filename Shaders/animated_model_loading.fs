#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Norm;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform vec3 light_pos;

void main()
{    
    float diffuse = abs(dot(Norm, light_pos));
    FragColor = texture(texture_diffuse, TexCoords) * diffuse;
}

