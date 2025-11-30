#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "Ray.h"
#include "Renderer.h"
#include "ModelTriangle.h"
#include "DrawingWindow.h"
#include "PointLight.h"

class RayTracer {
public:
	void resetZBuffer() {};
	void drawAllTriangles(
		DrawingWindow& window, 
		Buffers& buffers, 
		const std::vector<ModelTriangle>& triangles,
		glm::vec3& camera
	);
	void updateMatrices(glm::mat4& viewMat, glm::mat4& projMat);
	RayTracer(int w, int h);
	~RayTracer();

private:
	int width, height;
	glm::mat4 invPV; // Inverse of Projection * View matrix
	PointLight topLight = PointLight(glm::vec3(0.0f, 100.0f, 60.0f), glm::vec3(1.0, 1.0, 1.0), 2000.0f, "TopLight");

	const float ambient = 0.2f;
	// bool findIntersection(glm::vec3 cameraPosition, ModelTriangle triangle, Ray ray, float& T);
	bool findIntersection(const ModelTriangle& triangle, const Ray& ray, float& t);
	bool findNearestIntersection(
		const Ray& ray,
		const std::vector<ModelTriangle>& triangles,
		ModelTriangle& outTriangle,
		glm::vec3& outIntersectionPoint,
		float& outT
	);
};
#endif