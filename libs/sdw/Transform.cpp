#include "Transform.h"
#include "ModelTriangle.h"

#ifndef WIDTH
#define WIDTH 640
#endif

#ifndef HEIGHT
#define HEIGHT 480
#endif

ModelTriangle Transform::transformTriangle(ModelTriangle& triangle, glm::mat4& mat) {
	ModelTriangle transformedTriangle = triangle;
    for (int i = 0; i < 3; i++) {
        glm::vec4 homogenousCoord = glm::vec4(triangle.vertices[i], 1.0f);
        glm::vec4 transformedCoord = mat * homogenousCoord;
        transformedTriangle.vertices[i] = glm::vec3(transformedCoord) / transformedCoord.w;
    }
	return transformedTriangle;
}

std::vector<ModelTriangle> Transform::transformTriangles(std::vector<ModelTriangle> triangles, glm::mat4& mat) {
    std::vector<ModelTriangle> transformedTriangles;
    for (ModelTriangle t : triangles) {
        transformedTriangles.push_back(Transform::transformTriangle(t, mat));
    }
    return transformedTriangles;
}

std::vector<ModelTriangle> Transform::projectTriangles(std::vector<ModelTriangle> &triangles, mat4& viewMat, mat4& projMat) {
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
            if (clipSpacePos.w <= 0) {
                flag = true; 
                break;
            }
			// perspective divide for texture coordinates
			double inverseW = 1.0f / clipSpacePos.w;
            // NDC
            vec3 ndc = glm::vec3(clipSpacePos) * (float)inverseW;
            // store 1/w for perspective correction
			t.inverseW[i] = inverseW;
            // outside depth range (remove entire triangle)
			if (ndc.z < -1 || ndc.z > 1) {
                flag = true; 
                break;
            }
            // Screen space
			float sx = (ndc.x + 1) * 0.5f * WIDTH; // (-1 to 1) to (0 to WIDTH)
			float sy = (1 - ndc.y) * 0.5f * HEIGHT;// (-1 to 1) to (0 to HEIGHT)
            t.vertices[i] = glm::vec3(sx, sy, viewPos.z);
        }
        if (!flag) projectedTriangles.push_back(t);
    }
    return projectedTriangles;
}

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

void Transform::left(float degrees, vec3& camera, vec3& up) {
    mat3 R = rotate(-degrees, glm::normalize(up));
    camera = R * camera;
}

void Transform::up(float degrees, vec3& camera, vec3& up) {
    mat3 R = rotate(-degrees, glm::normalize(glm::cross(camera, up)));
    camera = R * camera;
    up = R * up;
}

// Dolly
void Transform::moveForward(float distance, vec3& camera, vec3& center, vec3& up) {
	vec3 forward = glm::normalize(center - camera);
	camera += forward * distance;
	center += forward * distance;
}

// Pedestal
void Transform::moveUp(float distance, vec3& camera, vec3& center, vec3& up) {
	vec3 u = glm::normalize(up);
	camera += u * distance;
	center += u * distance;
}

// Truck
void Transform::moveRight(float distance, vec3& camera, vec3& center, vec3& up) {
	vec3 right = glm::normalize(glm::cross(center - camera, up));
	camera += right * distance;
	center += right * distance;
}

Transform::Transform() {
}

Transform::~Transform() {
}