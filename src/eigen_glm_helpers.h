#pragma once

#include <glm/glm.hpp>
#include <Eigen/Dense>

#include <vector>

Eigen::Vector3f ConvertGLMVec3ToEigenVec3(const glm::vec3& from)
{
	Eigen::Vector3f to;
	to(0) = from.x;
	to(1) = from.y;
	to(2) = from.z;
	return to;
}

Eigen::MatrixXf MakeEigenMatrixWithGLMVec3Cols(const std::vector<glm::vec3>& cols) {
	Eigen::MatrixXf to(3, cols.size());
	for (int i = 0; i < cols.size(); i++) {
		to(0, i) = cols[i].x;
		to(1, i) = cols[i].y;
		to(2, i) = cols[i].z;
	}
	return to;
}
