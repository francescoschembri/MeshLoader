#pragma once

constexpr int MAX_BONE_INFLUENCE = 4;
constexpr int MAX_NUM_BONE = 100;

struct VertexBoneData {
	//bone indexes which will influence this vertex
	int BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float Weights[MAX_BONE_INFLUENCE];
	//num bones
	int NumBones = 0;

	friend bool operator==(const VertexBoneData& d1, const VertexBoneData& d2)
	{
		bool res = d1.NumBones == d2.NumBones;
		if (!res) return res;
		for (int i = 0; i < d1.NumBones; i++) {
			res = res && d1.BoneIDs[i] == d2.BoneIDs[i] && d1.Weights[i] == d2.Weights[i];
		}
		return res;
	}
};