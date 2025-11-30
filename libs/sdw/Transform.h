#ifndef TRANSFORM_H
#define TRANSFORM_H

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

struct ModelTriangle; // Forward declaration

class Transform
{
public:
	Transform();
	virtual ~Transform();

	static mat3 rotate(const float degrees, const vec3& axis);
	static void left(float degrees, vec3& camera, vec3& up);
	static void up(float degrees, vec3& camera, vec3& up);
	static void yaw(float degrees, vec3& camera, vec3& center, vec3& up);
	static void pitch(float degrees, vec3& camera, vec3& center, vec3& up);
	static void moveForward(float distance, vec3& camera, vec3& center, vec3& up);
	static void moveRight(float distance, vec3& camera, vec3& center, vec3& up);
	static void moveUp(float distance, vec3& camera, vec3& center, vec3& up);
	static ModelTriangle transformTriangle(ModelTriangle& triangle, mat4& mat);
	static std::vector<ModelTriangle> Transform::transformTriangles(std::vector<ModelTriangle> triangles, glm::mat4& mat);
	static std::vector<ModelTriangle> projectTriangles(std::vector<ModelTriangle>& triangles, glm::mat4& viewMat, glm::mat4& projMat);
};

#endif