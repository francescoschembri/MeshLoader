#include "Change.h"
#include "eigen_glm_helpers.h"

Change::Change(std::vector<Vertex*>& changedVertices)
	:
	changedVertices(changedVertices),
	offset(glm::vec3(0.0f, 0.0f, 0.0f))
{}

void Change::Apply() {
	for (auto&& v : changedVertices) {
		v->Position += offset;
		//glm::mat4 inverse = glm::inverse(v->associatedWeightMatrix);
		//v->originalVertex->Position += glm::vec3((inverse * glm::vec4(offset, 0.0f)));
	}
}

void Change::Undo() {
	for (auto&& v : changedVertices) {
		v->Position -= offset;
		//glm::mat4 inverse = glm::inverse(v->associatedWeightMatrix);
		//v->originalVertex->Position -= glm::vec3((inverse * glm::vec4(offset, 0.0f)));
	}
}

void Change::Modify(glm::vec3 newoffset)
{
	offset = newoffset - offset;
	Apply();
	offset = newoffset;
}


void Change::Reskin(std::vector<glm::mat4>& matrices) {
	for (auto&& v : changedVertices) {
		std::vector<glm::vec3> positions(matrices.size());
		for (int i = 0; i < matrices.size(); i++) {
			positions.push_back(glm::vec3(matrices[i] * glm::vec4(v->originalVertex->Position, 1.0f)));
		}
		Eigen::MatrixXf mat = MakeEigenMatrixWithGLMVec3Cols(positions);
		Eigen::Vector3f finalPos = ConvertGLMVec3ToEigenVec3(v->Position);
		Eigen::VectorXf weights = mat.colPivHouseholderQr().solve(finalPos);

		Vertex* original = v->originalVertex;
		original->BoneData.NumBones = 0;
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			if (weights(i) > FLT_EPSILON || weights(i) < -FLT_EPSILON) //!= 0 for floating point values
			{
				original->BoneData.BoneIDs[original->BoneData.NumBones] = i;
				original->BoneData.Weights[original->BoneData.NumBones++] = weights(i);
			}
		}
	}
}