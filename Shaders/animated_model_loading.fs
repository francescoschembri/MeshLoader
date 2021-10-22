#version 330 core
out vec4 FragColor;

flat in vec2 TexCoords;
flat in int Selected;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;
uniform sampler2D texture_specular3;
uniform bool wireframe;

void main()
{    
    if(wireframe)
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    else if(Selected == 0)
        FragColor = texture(texture_diffuse1, TexCoords);
    else
        FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

