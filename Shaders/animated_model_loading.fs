#version 330 core
out vec4 FragColor;

flat in vec3 TexCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_specular3;
uniform bool wireframe;

void main()
{    
    if(wireframe || TexCoords.z == -1.0f)
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    else if (TexCoords.z == 0.0f)
        FragColor = texture(texture_diffuse1, vec2(TexCoords.x, TexCoords.y) );
    else
        FragColor = texture(texture_diffuse2, vec2(TexCoords.x, TexCoords.y));
}

