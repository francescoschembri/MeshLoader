#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Norm;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

const vec3 LIGHT_POS = vec3(0.0, 0.0, 1.0);

void main()
{    
    float diffuse = abs(dot(Norm, LIGHT_POS));
    FragColor = texture(texture_diffuse, TexCoords) * diffuse;
}

