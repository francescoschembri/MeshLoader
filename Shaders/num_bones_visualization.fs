#version 330 core
out vec4 FragColor;

flat in int NumBones;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;


void main()
{    
    if (NumBones == 0)
    FragColor = vec4(0,0,0,1);
    if (NumBones == 1)
    FragColor = vec4(1,0,0,1);
    if (NumBones == 2)
    FragColor = vec4(1,1,0,1);
    if (NumBones == 3)
    FragColor = vec4(1,0,1,1);
    if (NumBones == 4)
    FragColor = vec4(0,1,0,1);
}

