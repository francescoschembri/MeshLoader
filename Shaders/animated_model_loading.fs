#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
flat in vec3 Norm;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform bool wireframe;

const vec3 LIGHT_POS = vec3(0.0, 0.0, 1.0);

void main()
{    
    float diffuse = abs(dot(Norm, LIGHT_POS));
    if(wireframe)
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    else
        FragColor = texture(texture_diffuse, TexCoords) * diffuse;
}

