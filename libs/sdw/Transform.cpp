#include "Transform.h"
#include "ModelTriangle.h"

#ifndef WIDTH
#define WIDTH 640
#endif

#ifndef HEIGHT
#define HEIGHT 480
#endif

mat3 Transform::rotate(const float degrees, const vec3& axis) {
    mat3 R; // rotation matrix
    mat3 K; // cross product matrix
    K = mat3(0, -axis.z, axis.y,
        axis.z, 0, -axis.x,
        -axis.y, axis.x, 0);
	float radians = glm::radians(degrees);
    R = mat3(1.0) + sin(radians) * K + (1 - cos(radians)) * K * K;
    return R;
}
// orbit
void Transform::left(float degrees) {
    vec3 offset = camera - center;
    offset = rotate(degrees, cameraUp) * offset;
    camera = center + offset;
    updateBasis();
}
// orbit
void Transform::up(float degrees) {
    vec3 axis = glm::normalize(glm::cross(cameraForward, cameraUp));
    vec3 offset = camera - center;
    offset = rotate(-degrees, axis) * offset;
    camera = center + offset;
    cameraUp = rotate(-degrees, axis) * cameraUp;
    updateBasis();
}

// direct rotate left-right
void Transform::yaw(float degrees) {
    mat3 R = rotate(degrees, cameraUp);
    cameraForward = glm::normalize(R * cameraForward);
    cameraRight = glm::normalize(glm::cross(cameraForward, cameraUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
    center = camera + cameraForward;
    updateBasis();
}

// direct rotate up-down
void Transform::pitch(float degrees) {
    mat3 R = rotate(degrees, cameraRight);
    cameraForward = glm::normalize(R * cameraForward);
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
    cameraRight = glm::normalize(glm::cross(cameraForward, cameraUp));
    center = camera + cameraForward;
    updateBasis();
}

// Dolly
void Transform::moveForward(float distance) {
    camera += cameraForward * distance;
    center += cameraForward * distance;
}
// Pedestal
void Transform::moveUp(float distance) {
    camera += cameraUp * distance;
    center += cameraUp * distance;
}
// Truck
void Transform::moveRight(float distance) {
    camera += cameraRight * distance;
    center += cameraRight * distance;
}

// Rolling
void Transform::roll(float degrees) {
    mat3 R = rotate(degrees, cameraForward);
    cameraUp = glm::normalize(R * cameraUp);
    cameraRight = glm::normalize(glm::cross(cameraForward, cameraUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
    center = camera + cameraForward;
}

// Update
void Transform::updateBasis() {
    cameraForward = glm::normalize(center - camera);
    cameraRight = glm::normalize(glm::cross(cameraForward, cameraUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
}

Transform::Transform(
    glm::vec3& cameraIn,
    glm::vec3& centerIn,
    glm::vec3& cameraUpIn,
    glm::vec3& cameraRightIn,
    glm::vec3& cameraForwardIn
)
    : camera(cameraIn),
    center(centerIn),
    cameraUp(cameraUpIn),
    cameraRight(cameraRightIn),
    cameraForward(cameraForwardIn)
{
    cameraForward = glm::normalize(center - camera);
    cameraRight = glm::normalize(glm::cross(cameraForward, cameraUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));
}

Transform::~Transform() = default;