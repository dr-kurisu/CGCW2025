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

class Transform
{
public:
	glm::vec3& camera;
	glm::vec3& center;
	glm::vec3& cameraUp;
	glm::vec3& cameraRight;
	glm::vec3& cameraForward;

public:
	Transform(vec3& cameraIn, vec3& centerIn, vec3& cameraUpIn, vec3& cameraRightIn, vec3& cameraForwardIn);
	~Transform();

	static mat3 rotate(const float degrees, const vec3& axis);
	void left(float degrees);
	void up(float degrees);
	void yaw(float degrees);
	void pitch(float degrees);
	void roll(float degrees);
	void moveForward(float distance);
	void moveRight(float distance);
	void moveUp(float distance);
	void updateBasis();
};

#endif