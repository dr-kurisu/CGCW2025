#ifndef MODEL_TRIANGLE_H
#define MODEL_TRIANGLE_H

#include "Material.h"
#include "Colour.h"
#include "TexturePoint.h"
#include <glm/glm.hpp>
#include <string>
#include <array>


struct ModelTriangle {
	std::array<glm::vec3, 3> vertices{};
	// std::array<TexturePoint, 3> texturePoints{};

	glm::vec3 u{};
	glm::vec3 v{};
	glm::vec3 inverseW = {1.0f, 1.0f, 1.0f};
	Colour colour{};
	glm::vec3 normal{};
	Material material{};
	
	ModelTriangle();
	ModelTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, Colour trigColour);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);

	// Has material with texture map
	bool hasTexture() const { return !material.mapName.empty(); }
};

#endif