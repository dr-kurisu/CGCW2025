#ifndef UTILS_H
#define UTILS_H
#include "ModelTriangle.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "PointLight.h"

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

std::vector<std::string> split(const std::string &line, char delimiter);

glm::vec3 convertToBarycentricCoordinates(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 r);

std::vector<PointLight> generateAreaLight(
	const glm::vec3& leftTop,
	const glm::vec3& rightTop,
	const glm::vec3& leftBottom,
	const glm::vec3& rightBottom,
	int samplesX,
	int samplesY,
	const glm::vec3& colour,
	float intensity,
	const std::string& name
);

void setReflectStrength(
	std::vector<ModelTriangle>& triangles,
	float strength
);

void printLights(const std::vector<PointLight>& lights);

ModelTriangle transformTriangle(ModelTriangle& triangle, mat4& mat);
std::vector<ModelTriangle> transformTriangles(std::vector<ModelTriangle> triangles, mat4& mat);
std::vector<ModelTriangle> projectTriangles(std::vector<ModelTriangle>& triangles, mat4& viewMat, mat4& projMat);

#endif

