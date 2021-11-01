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

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
	
out vec2 TexCoords;
flat out vec3 Norm;
	
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
    Norm = normalize((modelView * cumulativeMatrix * vec4(norm, 0.0)).xyz);
    TexCoords = tex;
}