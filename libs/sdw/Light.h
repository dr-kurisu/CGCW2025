// Light.h
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <string>

class Light {
public:
    std::string name;
    glm::vec3 colour;     
    float intensity;

    Light(const glm::vec3& colour = glm::vec3(1.0f),
        float intensity = 1.0f,
        const std::string& name = "")
        : name(name), colour(colour), intensity(intensity) {
    }

    virtual ~Light() = default;

    virtual glm::vec3 directionFrom(const glm::vec3& point) const = 0;

    virtual float distanceFrom(const glm::vec3& point) const = 0;

    virtual float attenuation(const glm::vec3& point) const {
        float d = distanceFrom(point);
        return 1.0f / (d * d);
    }
};

#endif
