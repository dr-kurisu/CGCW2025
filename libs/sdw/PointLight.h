#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "Light.h"

class PointLight : public Light {
public:
	glm::vec3 position;
	PointLight(const glm::vec3& position,
		const glm::vec3& colour = glm::vec3(1.0f),
		float intensity = 1.0f,
		const std::string& name = "")
		: Light(colour, intensity, name), position(position) {
	}
	glm::vec3 directionFrom(const glm::vec3& point) const override {
		return glm::normalize(position - point);
	}
	float distanceFrom(const glm::vec3& point) const override {
		return glm::length(position - point);
	}

};


#endif