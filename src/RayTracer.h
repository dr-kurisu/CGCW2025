#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "Ray.h"
#include "Renderer.h"
#include "ModelTriangle.h"
#include "DrawingWindow.h"
#include "PointLight.h"
#include <random>

class RayTracer {
public:
	void resetZBuffer() {};
	void drawAllTriangles(
		DrawingWindow& window, 
		const std::unordered_map<std::string, TextureMap>& allTextureMaps,
		const std::vector<ModelTriangle>& triangles
	);
	void updateMatrices(glm::mat4& viewMat, glm::mat4& projMat);
	void updateCamera(glm::vec3& camPos, glm::vec3& camRight, glm::vec3& camUp, glm::vec3& camForward) {
		camera = camPos; 
		camRight = camRight;
		camUp = camUp;
		camForward = camForward;
	}
	void updateLights(std::vector<PointLight>& inLights) { lights = inLights; }

	void loadEnvMap(TextureMap& textureMap) {
		envMap = textureMap;
	}

	RayTracer(int w, int h);
	~RayTracer();

private:
	int width, height;
	glm::mat4 invPV; // Inverse of Projection * View matrix
	std::vector<PointLight> lights = {};
	glm::vec3 camera = {};
	glm::vec3 camRight, camUp, camForward = {};

	const float ambient = 0.2f;
	float shininess = 32.0f;
	float specStrength = 0.7f;
	float attenuationFactor = 250.0f;

	float focusDist = 200.0f;  // focus length
	float lensRadius = 0.0f;    //len radius
	int sppDOF = 1;       // sample point per pixel for DOF

	bool useEnvMap = true;

	TextureMap& envMap = TextureMap();
	
	bool isInShadow(const glm::vec3& pos, const PointLight& light, const std::vector<ModelTriangle>& triangles);

	Colour computePhongLighting(
		const glm::vec3& pos,
		const std::vector<ModelTriangle>& triangles,
		const glm::vec3& normal,
		const Colour& baseColour,
		float metallic = 0.0f
	);

	// bool findIntersection(glm::vec3 cameraPosition, ModelTriangle triangle, Ray ray, float& T);
	bool findIntersection(const ModelTriangle& triangle, const Ray& ray, float& t);
	bool findNearestIntersection(
		const Ray& ray,
		const std::vector<ModelTriangle>& triangles,
		ModelTriangle& outTriangle,
		glm::vec3& outIntersectionPoint,
		float& outT
	);

	glm::vec3 traceRay(
		const Ray& ray,
		const std::vector<ModelTriangle>& triangles,
		const std::unordered_map<std::string, TextureMap>& allTextureMaps,
		int depth
	);

	void RenderRows(
		int yStart, int yEnd,
		const std::vector<ModelTriangle>& triangles,
		const std::unordered_map<std::string, TextureMap>& allTextureMaps,
		std::vector<uint32_t>& colourBuffer
	);

	static glm::vec2 randomInUnitDisk(std::mt19937& rng) {
		std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
		while (true) {
			float x = dist(rng);
			float y = dist(rng);
			if (x * x + y * y <= 1.0f) return { x, y };
		}
	}
};
#endif