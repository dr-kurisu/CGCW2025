#include "FileLoader.h"

TextureMap readMTLFile(const std::string& filePath) {
	// Open the file
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return {};
	}

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		std::string prefix;
		std::istringstream lineStream(line);

		lineStream >> prefix;
		if (prefix == "map_Kd") {
			std::string textureFilename;
			lineStream >> textureFilename;
			std::string texturePath = "./models/" + textureFilename;
			return TextureMap(texturePath);
		}
	}
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
			while (lineStream >> point) {
				int seperator = point.find('/');
				int vertexIndex = std::stoi(point.substr(0, seperator)) - 1;
				int textureIndex = stoi(point.substr(seperator + 1)) - 1;
				buffers.indices.push_back(vertexIndex);
				buffers.textureIndices.push_back(textureIndex);
			}
		}
		else if (prefix == "mtllib") {
			std::string filename;
			lineStream >> filename;
			std::string MTLPath = "./models/" + filename;
			buffers.textureMap = readMTLFile(MTLPath);
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
		int vt0 = uvIndices[i * 3 + 0];
		int vt1 = uvIndices[i * 3 + 1];
		int vt2 = uvIndices[i * 3 + 2];

		TexturePoint t0 = TexturePoint(textureCoords[vt0].x, textureCoords[vt0].y);
		TexturePoint t1 = TexturePoint(textureCoords[vt1].x, textureCoords[vt1].y);
		TexturePoint t2 = TexturePoint(textureCoords[vt2].x, textureCoords[vt2].y);
		// Calculate normal by cross product
		triangle.vertices = { vertices[v0], vertices[v1], vertices[v2]};
		triangle.texturePoints = { t0, t1, t2 };

		// Maybe need to reverse (depend on the order of vertex)
		triangle.normal = glm::normalize(glm::cross(vertices[v1] - vertices[v0], vertices[v2] - vertices[v0]));

		result.push_back(triangle);
	}
	return result;
}