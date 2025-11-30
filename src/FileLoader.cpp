#include "FileLoader.h"

void readMTLFile(Buffers &buffers, const std::string& filePath) {
	// Open the file
	std::unordered_map<std::string, Material> materials;

	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return;
	}

	Material* currentMaterial = nullptr;
	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		std::string prefix;
		std::istringstream lineStream(line);
		lineStream >> prefix;
		// Read newmtl
		if (prefix == "newmtl") {
			std::string materialName;
			lineStream >> materialName;
			materials[materialName] = Material(materialName);
			currentMaterial = &materials[materialName];
		}
		// Read Map_Kd
		else if (prefix == "map_Kd" && currentMaterial) {
			std::string textureFilename;
			lineStream >> textureFilename;
			std::string texturePath = "./models/" + textureFilename;
			currentMaterial->type = "kd_map";
			currentMaterial->mapName = textureFilename;
			if (buffers.textureMaps.find(textureFilename) == buffers.textureMaps.end()) {
				buffers.textureMaps.emplace(textureFilename, TextureMap(texturePath));
			}
		}
		// Read Kd
		else if (prefix == "Kd" && currentMaterial) {
			float r, g, b;
			lineStream >> r >> g >> b;
			currentMaterial->type = "kd";
			currentMaterial->Kd = Colour(
				static_cast<int>(r * 255),
				static_cast<int>(g * 255),
				static_cast<int>(b * 255)
			);
		}
	}
	buffers.materials = materials;
}

Buffers readOBJFile(const std::string &filePath, float factor) {
	// Open the file
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return {};
	}

	std::string line;
	Buffers buffers;
	std::unordered_map<std::string, Material> materials;
	Material currentMaterial = Material();  // default material

	while (std::getline(file, line)) {
		if (line.empty()) continue;
		// deal with prefix
		std::string prefix;
		std::istringstream lineStream(line);
		lineStream >> prefix;

		if (prefix == "o") {
			buffers.name = lineStream.str().substr(2);
		}
		else if (prefix == "v") {
			float x, y, z;
			lineStream >> x >> y >> z;
			buffers.vertices.emplace_back(x * factor, y * factor, z * factor);
		}
		else if (prefix == "vt") {
			float u, v;
			lineStream >> u >> v;
			buffers.textureCoords.emplace_back(u, v);
		}
		else if (prefix == "f") {
			std::string point;
			int vertexIndices[3];
			int texIndices[3] = { -1, -1, -1 };
			int count = 0;

			while (lineStream >> point && count < 3) {
				int vIndex = -1;
				int tIndex = -1;

				size_t firstSlash = point.find('/');
				size_t secondSlash = point.find('/', firstSlash + 1);

				// case 1: "v/"
				if (firstSlash == point.size() - 1) {
					vIndex = std::stoi(point) - 1;
				}
				// case 2: "v/vt"
				else if (secondSlash == std::string::npos) {
					vIndex = std::stoi(point.substr(0, firstSlash)) - 1;
					tIndex = std::stoi(point.substr(firstSlash + 1)) - 1;
				}
				vertexIndices[count] = vIndex;
				texIndices[count] = tIndex;
				count++;
			}
			buffers.indices.push_back(vertexIndices[0]);
			buffers.indices.push_back(vertexIndices[1]);
			buffers.indices.push_back(vertexIndices[2]);

			buffers.textureIndices.push_back(texIndices[0]);
			buffers.textureIndices.push_back(texIndices[1]);
			buffers.textureIndices.push_back(texIndices[2]);

			buffers.faceMaterialNames.push_back(currentMaterial.name);
		}
		else if (prefix == "mtllib") {
			std::string filename;
			lineStream >> filename;
			std::string MTLPath = "./models/" + filename;
			readMTLFile(buffers, MTLPath);
		}
		else if (prefix == "usemtl") {
			std::string materialName;
			lineStream >> materialName;
			if (buffers.materials.find(materialName) != buffers.materials.end()) {
				currentMaterial = buffers.materials[materialName];
			}
		}

		else {
			std::cout << "Unknown line: " << line << std::endl;
		}
	}
	return buffers;
}

std::vector<ModelTriangle> convertToTriangles(const Buffers& buffers) {
	std::vector<ModelTriangle> result;
	auto& vertices = buffers.vertices;
	auto& indices = buffers.indices;
	auto& uvIndices = buffers.textureIndices;
	auto& textureCoords = buffers.textureCoords;

	int count = indices.size() / 3;
	for (int i = 0; i < count; i++) {
		ModelTriangle triangle = ModelTriangle();
		int v0 = indices[i * 3 + 0];
		int v1 = indices[i * 3 + 1];
		int v2 = indices[i * 3 + 2];

		triangle.vertexIndices = { v0, v1, v2 };
		std::string materialName = buffers.faceMaterialNames[i]; // assume each triangle has same material
		if (buffers.materials.find(materialName) != buffers.materials.end()) {
			triangle.material = buffers.materials.at(materialName);
		}else {
			triangle.material = Material();
		}

		if (triangle.material.type == "kd_map") {
			// if there is no texture coords, skip
			int vt0 = uvIndices[i * 3 + 0];
			int vt1 = uvIndices[i * 3 + 1];
			int vt2 = uvIndices[i * 3 + 2];
			triangle.u = glm::vec3(textureCoords[vt0].x, textureCoords[vt1].x, textureCoords[vt2].x);
			triangle.v = glm::vec3(textureCoords[vt0].y, textureCoords[vt1].y, textureCoords[vt2].y);
		}
		// No need to deal with kd colour
		// Calculate normal by cross product
		triangle.vertices = { vertices[v0], vertices[v1], vertices[v2]};
		result.push_back(triangle);
	}
	return result;
}

void computeNormals(Buffers& buffers, std::vector<ModelTriangle>& triangles) {
	// need init
	buffers.vertexNormals.assign(buffers.vertices.size(), glm::vec3(0.0f));
	for (auto& t : triangles) {
		glm::vec3 v0 = t.vertices[0];
		glm::vec3 v1 = t.vertices[1];
		glm::vec3 v2 = t.vertices[2];
		
		glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
		float area = 0.5f * glm::length(glm::cross(v1 - v0, v2 - v0));
		buffers.vertexNormals[t.vertexIndices[0]] += faceNormal * area;
		buffers.vertexNormals[t.vertexIndices[1]] += faceNormal * area;
		buffers.vertexNormals[t.vertexIndices[2]] += faceNormal * area;
		t.normal = faceNormal;
	}
	// Normalise
	for (auto& n : buffers.vertexNormals) {
		n = glm::normalize(n);
	}
	// Assign
	for (auto& triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			int index = triangle.vertexIndices[i];
			triangle.vertexNormals[i] = buffers.vertexNormals[index];
		}
	}
}

void printBuffers(const Buffers &buffers) {
	std::cout << "===== OBJ Buffers Summary =====\n";
	std::cout << "Object name: " << buffers.name << "\n\n";
	std::cout << "Vertices count: " << buffers.vertices.size() << "\n";
	std::cout << "Texture coords count: " << buffers.textureCoords.size() << "\n";
	std::cout << "Triangle index count: " << buffers.indices.size()
		<< "  (triangles: " << buffers.indices.size() / 3 << ")\n";
	std::cout << "Faces with materials: " << buffers.faceMaterialNames.size() << "\n";
	std::cout << "\nMaterials (" << buffers.materials.size() << "):\n";
	for (const auto& pair : buffers.materials) {
		std::cout << "  - " << pair.first << "\n";
	}

	std::cout << "\nTextureMaps (" << buffers.textureMaps.size() << "):\n";
	for (const auto& pair : buffers.textureMaps) {
		std::cout << "  - " << pair.first << "\n";
	}

	std::cout << "================================\n";
}