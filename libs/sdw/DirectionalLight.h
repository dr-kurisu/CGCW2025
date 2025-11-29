#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "Light.h"

class DirectionalLight : public Light {
public:
    glm::vec3 direction;

    DirectionalLight(const glm::vec3& dir,
        const glm::vec3& colour = glm::vec3(1.0f),
        float intensity = 1.0f,
        const std::string& name = "")
        : Light(colour, intensity, name), direction(glm::normalize(dir)) {
    }

    glm::vec3 directionFrom(const glm::vec3& point) const override {
        return -direction;
    }

    float distanceFrom(const glm::vec3& point) const override {
        return std::numeric_limits<float>::infinity();
    }

    float attenuation(const glm::vec3& point) const override {
        return 1.0f;
    }
};

#endif