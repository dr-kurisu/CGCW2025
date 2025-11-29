#include "RayTracer.h"
#include "ModelTriangle.h"


bool RayTracer::findIntersection(glm::vec3 cameraPosition, ModelTriangle triangle, Ray ray, float& T){
	glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
	glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
	glm::vec3 SPVector = cameraPosition - triangle.vertices[0];
	glm::mat3 DEMatrix(-ray.direction, e0, e1);
	glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
	float t = possibleSolution.x, u = possibleSolution.y, v = possibleSolution.z;
	if (t > 0 && u >= 0 && v >= 0 && (u + v) <= 1) {
		T = t;
		return true;
	}
	return false;
}

bool RayTracer::findNearestIntersection(
	const glm::vec3& cameraPosition,
	const Ray& ray,
	const std::vector<ModelTriangle>& triangles,
	ModelTriangle& outTriangle,
	glm::vec3& outIntersectionPoint,
	float& outT
) {
	bool hit = false;
	float minT = std::numeric_limits<float>::max();
	ModelTriangle closestTriangle;

	for (const auto& triangle : triangles) {
		float T;
		if (findIntersection(cameraPosition, triangle, ray, T)) {
			if (T < minT) {
				minT = T;
				closestTriangle = triangle;
				hit = true;
			}
		}
	}

	if (hit) {
		outTriangle = closestTriangle;
		outT = minT;
		outIntersectionPoint = ray.at(minT);
	}

	return hit;
}

void RayTracer::drawAllTriangles(DrawingWindow& window, Buffers& buffers, const std::vector<ModelTriangle>& triangles) {
	// Need to update projectMat and ViewMat
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float xNDC = (2.0f * (x + 0.5f) / width - 1.0f);
			float yNDC = (1.0f - 2.0f * (y + 0.5f) / height);
			glm::vec4 nearPointClip(xNDC, yNDC, -1.0f, 1.0f);
			glm::vec4 farPointClip(xNDC, yNDC, 1.0f, 1.0f);
			glm::vec4 nearWorld = invPV * nearPointClip;
			glm::vec4 farWorld = invPV * farPointClip;
			nearWorld /= nearWorld.w;
			farWorld /= farWorld.w;

			glm::vec3 rayOrigin = glm::vec3(nearWorld);
			glm::vec3 rayDir = glm::normalize(glm::vec3(farWorld - nearWorld));
			Ray ray(rayOrigin, rayDir);

			ModelTriangle hitTriangle;
			glm::vec3 intersectionPoint;
			float t;
			if (findNearestIntersection(rayOrigin, ray, triangles, hitTriangle, intersectionPoint, t)) {
				// Compute normal
				glm::vec3 normal = glm::normalize(glm::cross(
					hitTriangle.vertices[1] - hitTriangle.vertices[0],
					hitTriangle.vertices[2] - hitTriangle.vertices[0]
				));
				glm::vec3 lightDir = topLight.directionFrom(intersectionPoint);
				glm::vec3 shadowOrigin = intersectionPoint + 0.001f * lightDir;
				Ray shadowRay(shadowOrigin, lightDir);
				ModelTriangle shadowTriangle;
				glm::vec3 shadowIntersectionPoint;
				float shadowT;
				bool inShadow = false;
				// Shadow check
				if (findNearestIntersection(shadowOrigin, shadowRay, triangles, shadowTriangle, shadowIntersectionPoint, shadowT)) {
					float lightDistance = glm::length(topLight.position - intersectionPoint);
					if (shadowT < lightDistance)
						inShadow = true;
				}
				// Lighting
				float diffuse = pointLightFactor(topLight, intersectionPoint, normal);
				float ambient = 0.3f; // ambient light
				float brightness = inShadow ? ambient : diffuse + ambient;
				brightness = glm::clamp(brightness, 0.0f, 1.0f);

				if (hitTriangle.material.type == "kd") {
					// KD Colour
					Colour pixelColour = hitTriangle.material.Kd;
					// Typecast to uint8_t
					uint8_t r = static_cast<uint8_t>(pixelColour.red * brightness);
					uint8_t g = static_cast<uint8_t>(pixelColour.green * brightness);
					uint8_t b = static_cast<uint8_t>(pixelColour.blue * brightness);

					uint32_t colourInt = (255 << 24) + (r << 16) + (g << 8) + b;
					window.setPixelColour(x, y, colourInt);
				}
			}
		}
	}
}

void RayTracer::updateMatrices(glm::mat4& viewMat, glm::mat4& projMat) {
	invPV = glm::inverse(projMat * viewMat);
}

float RayTracer::pointLightFactor(PointLight light, glm::vec3 intersectionPoint, glm::vec3& normal) {
	glm::vec3 lightDir = light.directionFrom(intersectionPoint);
	float lightDist = light.distanceFrom(intersectionPoint);
	float attenuation = 1.0f;
	float diffuse = glm::dot(normal, lightDir);
	diffuse = glm::clamp(diffuse, 0.0f, 1.0f);
	return diffuse * light.intensity * attenuation;
}


RayTracer::RayTracer(int w, int h) 
	:width(w), height(h)
{}

RayTracer::~RayTracer() {}