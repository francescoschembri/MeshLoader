#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec2 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;
layout(location = 7) in int numBones;
layout(location = 8) in int selected;
	
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
	
out vec2 TexCoords;
out int Selected;
flat out vec3 Norm;
	
void main()
{
    vec4 totalPosition = vec4(pos,1.0);
    vec4 totalNormal = vec4(norm,0.0);
    if(numBones>0) {
        totalPosition = vec4(0.0f);
        totalNormal = vec4(0.0f);
    }
    for(int i = 0 ; i < numBones ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(pos,1.0f);
            totalNormal = vec4(norm,0.0f);
            break;
        }
        vec4 localNormal = finalBonesMatrices[boneIds[i]] * vec4(norm,0.0);
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos,1.0);
        totalPosition += localPosition * weights[i];
        totalNormal += localNormal * weights[i];
    }
    gl_Position =  projection * view * model * totalPosition;
    TexCoords = tex;
    Selected = selected;
    Norm = normalize((view * model * totalNormal).xyz);
}