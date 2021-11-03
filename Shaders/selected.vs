#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in ivec4 boneIds; 
layout(location = 2) in vec4 weights;
layout(location = 3) in int numBones;
	
uniform mat4 projection;
uniform mat4 modelView;
uniform mat4 finalBonesMatrices[100];

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
}  