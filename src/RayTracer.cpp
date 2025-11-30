#include "RayTracer.h"
#include "ModelTriangle.h"

//bool RayTracer::findIntersection(glm::vec3 cameraPosition, ModelTriangle triangle, Ray ray, float& T){
//	glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
//	glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
//	glm::vec3 SPVector = cameraPosition - triangle.vertices[0];
//	glm::mat3 DEMatrix(-ray.direction, e0, e1);
//	glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
//	float t = possibleSolution.x, u = possibleSolution.y, v = possibleSolution.z;
//	if (t > 0 && u >= 0 && v >= 0 && (u + v) <= 1) {
//		T = t;
//		return true;
//	}
//	return false;
//}

bool RayTracer::findIntersection(const ModelTriangle& triangle, const Ray& ray, float& t) {
	const float EPSILON = 1e-5f;
	glm::vec3 v0 = triangle.vertices[0];
	glm::vec3 v1 = triangle.vertices[1];
	glm::vec3 v2 = triangle.vertices[2];

	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;
	glm::vec3 h = glm::cross(ray.direction, edge2);
	float a = glm::dot(edge1, h);
	// Parallel
	if (fabs(a) < EPSILON) return false;

	float f = 1.0f / a;
	glm::vec3 s = ray.origin - v0;
	float u = f * glm::dot(s, h);
	if (u < 0.0f || u > 1.0f) return false;

	glm::vec3 q = glm::cross(s, edge1);
	float v = f * glm::dot(ray.direction, q);
	if (v < 0.0f || u + v > 1.0f) return false;

	t = f * glm::dot(edge2, q);
	return t > EPSILON;
}

bool RayTracer::findNearestIntersection(
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
		if (findIntersection(triangle, ray, T)) {
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

void RayTracer::drawAllTriangles(
	DrawingWindow& window, 
	Buffers& buffers, 
	const std::vector<ModelTriangle>& triangles,
	glm::vec3& camera
) {
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
			Ray ray(rayOrigin, rayDir); // Main ray

			ModelTriangle hitTriangle;
			glm::vec3 intersectionPoint;
			float t;
			if (findNearestIntersection(ray, triangles, hitTriangle, intersectionPoint, t)) {
				// Compute normal
				glm::vec3 normal = hitTriangle.normal;
				glm::vec3 lightDir = topLight.directionFrom(intersectionPoint);
				glm::vec3 viewDir = glm::normalize(camera - intersectionPoint);
				glm::vec3 shadowOrigin = intersectionPoint + 0.01f * lightDir; // prevent self-shadowing
				Ray shadowRay(shadowOrigin, lightDir);  // Shadow ray
				ModelTriangle shadowTriangle;
				glm::vec3 shadowIntersectionPoint;
				float shadowT;
				bool shadow = false;
				float lightDistance = glm::length(topLight.position - intersectionPoint);
				for (const auto& tri : triangles) {
					float shadowT;
					if (findIntersection(tri, shadowRay, shadowT) && shadowT < lightDistance) {
						shadow = true;
						break;  // Early exit if in shadow
					}
				}
				// Lighting
				float diffuse = 0.0f;
				float specular = 0.0f;
				if (!shadow) {
					// Angle of Incidence
					glm::vec3 lightDir = glm::normalize(topLight.position - intersectionPoint);
					diffuse = std::max(0.0f, glm::dot(normal, lightDir));
					// Proximity lighting
					float d = glm::length(topLight.position - intersectionPoint);
					float attenuation = 150.0f / (4.0f * M_PI * d * d);
					diffuse *= attenuation;
					// Specular
					glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
					float specAngle = std::max(0.0f, glm::dot(viewDir, reflectDir));
					float shininess = 64;
					float specStrength = 500.0f;
					specular = specStrength * std::pow(specAngle, shininess);
					specular *= attenuation;
				}

				float brightness = ambient + diffuse * topLight.intensity + specular;
				brightness = glm::clamp(brightness, 0.0f, 1.0f);

				if (hitTriangle.material.type == "kd") {
					// KD Colour
					Colour pixelColour = hitTriangle.material.Kd;
					// Typecast
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

RayTracer::RayTracer(int w, int h) 
	:width(w), height(h)
{}

RayTracer::~RayTracer() {}