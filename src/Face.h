#pragma once

#include <array>

struct Face {
	// indices of the face/triangle
    std::array<unsigned int, 3> indices;
    Face() = default;
    Face(const Face& other) = default;
    Face(const std::array<unsigned int, 3>& other_indices) : indices{ other_indices } {}

    friend Face operator+(const Face& f1, const Face& f2)
    {
        Face result{};
        for (unsigned int i = 0; i < 3; i++) {
            result.indices[i] = f1.indices[i] + f2.indices[i];
        }
        return result;
    }
};