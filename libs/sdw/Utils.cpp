#include <algorithm>
#include <sstream>
#include "Utils.h"

#ifndef WIDTH
#define WIDTH 640
#endif

#ifndef HEIGHT
#define HEIGHT 480
#endif

std::vector<std::string> split(const std::string &line, char delimiter) {
	auto haystack = line;
	std::vector<std::string> tokens;
	size_t pos;
	while ((pos = haystack.find(delimiter)) != std::string::npos) {
		tokens.push_back(haystack.substr(0, pos));
		haystack.erase(0, pos + 1);
	}
	// Push the remaining chars onto the vector
	tokens.push_back(haystack);
	return tokens;
}

// Uses Cramer’s rule to convert from 2D coordinates to Barycentric proportional proximities
glm::vec3 convertToBarycentricCoordinates(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 r)
{
    glm::vec3 e0 = v1 - v0;
    glm::vec3 e1 = v2 - v0;
    glm::vec3 e2 = r - v0;
    float d00 = glm::dot(e0, e0);
    float d01 = glm::dot(e0, e1);
    float d11 = glm::dot(e1, e1);
    float d20 = glm::dot(e2, e0);
    float d21 = glm::dot(e2, e1);
	// change order to make sure alpha, beta, gamma
    float denominator = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denominator;
    float w = (d00 * d21 - d01 * d20) / denominator;
    float u = 1.0f - v - w;
    return glm::vec3(u,v,w);
}

// Same Y
std::vector<PointLight> generateAreaLight(
    const glm::vec3& leftTop,
    const glm::vec3& rightTop,
    const glm::vec3& leftBottom,
    const glm::vec3& rightBottom,
    int samplesX,
    int samplesZ,
    const glm::vec3& colour,
    float intensity,
    const std::string& name
) {
    std::vector<PointLight> areaLights;
    for (int i = 0; i < samplesX; ++i) {
        for (int j = 0; j < samplesZ; ++j) {
            float stepX = float(i) / float(samplesX - 1);
            float stepZ = float(j) / float(samplesZ - 1);
			// interpolate along top and bottom edges
            glm::vec3 topEdge = leftTop + stepX * (rightTop - leftTop);
            glm::vec3 bottomEdge = leftBottom + stepX * (rightBottom - leftBottom);
			glm::vec3 position = bottomEdge + stepZ * (topEdge - bottomEdge);
            std::string lightName = name + "_(" + std::to_string(i) + "," + std::to_string(j) + ")";
			areaLights.emplace_back(position, colour, intensity / (samplesX * samplesZ), lightName);
        }
    }
    return areaLights;
}

void setReflectStrength(
    std::vector<ModelTriangle>& triangles,
	float strength
) {
    for (auto& triangle : triangles) {
		triangle.material.type = "reflect";
        triangle.material.reflectStrength = strength;
    }
}

void printLights(const std::vector<PointLight>& lights) {
    for (size_t i = 0; i < lights.size(); ++i) {
        const auto& light = lights[i];
        std::cout << "[" << i << "] "
            << light.name << std::endl;
        std::cout << "  Position : ("
            << light.position.x << ", "
            << light.position.y << ", "
            << light.position.z << ")" << std::endl;
        std::cout << "  Colour   : ("
            << light.colour.r << ", "
            << light.colour.g << ", "
            << light.colour.b << ")" << std::endl;
        std::cout << "  Intensity: " << light.intensity << std::endl;
    }
}

ModelTriangle transformTriangle(ModelTriangle& triangle, mat4& mat) {
    ModelTriangle transformedTriangle = triangle;
    for (int i = 0; i < 3; i++) {
        glm::vec4 homogenousCoord = glm::vec4(triangle.vertices[i], 1.0f);
        glm::vec4 transformedCoord = mat * homogenousCoord;
        transformedTriangle.vertices[i] = glm::vec3(transformedCoord) / transformedCoord.w;
    }
    return transformedTriangle;
}

std::vector<ModelTriangle> transformTriangles(std::vector<ModelTriangle> triangles, mat4& mat) {
    std::vector<ModelTriangle> transformedTriangles;
    for (ModelTriangle t : triangles) {
        transformedTriangles.push_back(transformTriangle(t, mat));
    }
    return transformedTriangles;
}

std::vector<ModelTriangle> projectTriangles(std::vector<ModelTriangle>& triangles, mat4& viewMat, mat4& projMat) {
    std::vector<ModelTriangle> projectedTriangles;
    // UV unchange
    for (ModelTriangle t : triangles) {
        bool flag = false;
        // Deep Copy not pointer copy
        for (int i = 0; i < 3; i++) {
            glm::vec3 worldCoord = t.vertices[i];
            // Tranformation to clip space
            vec4 viewPos = viewMat * vec4(worldCoord, 1.0f);
            vec4 clipSpacePos = projMat * viewPos;
            // behind the camera (remove entire triangle)
            if (clipSpacePos.w <= 1e-6) {
                flag = true;
                break;
            }
            // perspective divide for texture coordinates
            double inverseW = 1.0f / clipSpacePos.w;
            // NDC
            vec3 ndc = glm::vec3(clipSpacePos) * (float)inverseW;
            // store 1/w for perspective correction
            t.inverseW[i] = inverseW;
            // May outside depth range
            if (ndc.z < -1.0f || ndc.z > 1.0f) {
                flag = true;
                break;
            }
            // Screen space
            float sx = (ndc.x + 1) * 0.5f * WIDTH; // (-1 to 1) to (0 to WIDTH)
            float sy = (1 - ndc.y) * 0.5f * HEIGHT;// (-1 to 1) to (0 to HEIGHT)
            t.vertices[i] = glm::vec3(sx, sy, ndc.z);
        }
        if (!flag) projectedTriangles.push_back(t);
    }
    return projectedTriangles;
}
