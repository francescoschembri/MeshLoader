#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec2 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;
layout(location = 7) in int numBones;
	
uniform mat4 projection;
uniform mat4 modelView;
uniform mat4 finalBonesMatrices[100];

out vec4 aColor;
	
void main()
{
    mat4 cumulativeMatrix = mat4(1.0);
    if (numBones>0)
        cumulativeMatrix = mat4(0.0);
    for(int i = 0 ; i < numBones ; i++)
    {
        cumulativeMatrix += (finalBonesMatrices[boneIds[i]] * weights[i]);
    }
    gl_Position =  projection * modelView * cumulativeMatrix * vec4(pos, 1.0);

    if(numBones == 0)
        aColor = vec4(0, 0, 0, 1);
    else if(numBones == 1)
        aColor = vec4(1,0,0,1);
    else if(numBones == 2)
        aColor = vec4(1,0.5, 0.05, 1);
    else if(numBones == 3)
        aColor = vec4(1,1,0,1);
    else if(numBones == 4)
        aColor = vec4(0,1,0,1);
    else
        aColor = vec4(0, 0, 1, 1);
}