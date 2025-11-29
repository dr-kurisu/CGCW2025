#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

struct Ray{
	glm::vec3 origin;
	glm::vec3 direction;

	Ray() = default;

	Ray(const glm::vec3& o, const glm::vec3& d) 
		: origin(o), direction(d)
	{}

	glm::vec3 at(float t) const {
		return origin + t * direction;
	}
};

#endif
