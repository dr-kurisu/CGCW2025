#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "Ray.h"
#include "Renderer.h"
#include "ModelTriangle.h"
#include "DrawingWindow.h"
#include "PointLight.h"

class RayTracer : public Renderer {
public:
	void resetZBuffer() override {};
	void drawAllTriangles(DrawingWindow& window, Buffers& buffers, const std::vector<ModelTriangle>& triangles) override;
	void updateMatrices(glm::mat4& viewMat, glm::mat4& projMat);
	RayTracer(int w, int h);
	~RayTracer();

private:
	int width, height;
	glm::mat4 invPV;
	PointLight topLight = PointLight(glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(1.0, 1.0, 1.0), 500.0f, "TopLight");
	bool findIntersection(glm::vec3 cameraPosition, ModelTriangle triangle, Ray ray, float& T);
	bool findNearestIntersection(
		const glm::vec3& cameraPosition,
		const Ray& ray,
		const std::vector<ModelTriangle>& triangles,
		ModelTriangle& outTriangle,
		glm::vec3& outIntersectionPoint,
		float& outT
	);
	float pointLightFactor(PointLight light, glm::vec3 intersectionPoint, glm::vec3& normal);
};
#endif