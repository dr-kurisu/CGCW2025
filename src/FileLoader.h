#ifndef FILELOADER_H
#define FILELOADER_H

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "TextureMap.h"
#include "ModelTriangle.h"
#include "Material.h"


struct Buffers {
	std::string name;
	std::vector<glm::vec3> vertices;
	std::unordered_map<std::string, TextureMap> textureMaps;
	std::unordered_map<std::string, Material> materials;
	std::vector<std::string> faceMaterialNames;
	std::vector<glm::vec2> textureCoords;
	std::vector<int> indices;
	std::vector<int> textureIndices;
	std::vector<glm::vec3> vertexNormals;
};

Buffers readOBJFile(const std::string &filePath, float factor);

std::vector<ModelTriangle> convertToTriangles(const Buffers& buffers);

void printBuffers(const Buffers &buffers);

void computeNormals(Buffers& buffers, std::vector<ModelTriangle>& triangles);
#endif