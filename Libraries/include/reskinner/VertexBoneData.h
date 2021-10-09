#pragma once

constexpr int MAX_BONE_INFLUENCE = 100;

struct VertexBoneData {
	//bone indexes which will influence this vertex
	int BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float Weights[MAX_BONE_INFLUENCE];
	//num bones
	int NumBones = 0;
};