#include "RayTracer.h"
#include "ModelTriangle.h"
#include "Utils.h"
#include <thread>
#include <algorithm>

// Class verision
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
	const float EPSILON = 1e-6f;
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

glm::vec2 directionToUV(const glm::vec3& dir) {
	float theta = atan2(dir.z, dir.x); // in radians
	float phi = acos(glm::clamp(dir.y, -1.0f, 1.0f));
	float u = (theta + M_PI) / (2.0f * M_PI);
	float v = phi / M_PI;
	return glm::vec2(u, v);
}

Colour RayTracer::computePhongLighting(
	const glm::vec3& pos,
	const std::vector<ModelTriangle>& triangles,
	const glm::vec3& normal,
	const Colour& baseColour,
	float metallic
) {
	// Colour transformation
	glm::vec3 base = glm::vec3(baseColour.red, baseColour.green, baseColour.blue) / 255.0f;
	glm::vec3 total = glm::vec3(0.0f);
	for (auto light : lights) {
		bool shadow = isInShadow(pos, light, triangles);
		if (!shadow) {
			glm::vec3 lightDir = glm::normalize(light.position - pos);
			glm::vec3 viewDir = glm::normalize(camera - pos);
			glm::vec3 halfDir = glm::normalize(lightDir + viewDir);

			// Diffuse
			float diff = std::max(0.0f, glm::dot(normal, lightDir));
			// Specular
			float spec = pow(std::max(0.0f, dot(normal, halfDir)), shininess);

			float d = glm::length(light.position - pos);
			float attenuation = attenuationFactor / (4 * M_PI * d * d);

			glm::vec3 lightCol = light.colour * light.intensity * attenuation;
			glm::vec3 diffuse = diff * lightCol * base * (1.0f - metallic);
			// glm::vec3 specular = specStrength * spec * lightCol;
			glm::vec3 specular = spec * lightCol * glm::mix(
				glm::vec3(specStrength),   // Non-metal high light
				base,                      // Metal high light
				metallic
			);
			total += diffuse + specular;
		}
		else {
			// debug
		}
	}
	total += glm::vec3(ambient) * base; // ambient
	total = glm::clamp(total, 0.0f, 1.0f);
	return Colour(int(total.r * 255), int(total.g * 255), int(total.b * 255));
}

Colour vec3ToColour(const glm::vec3& colourVec) {
	return Colour(
		std::clamp(static_cast<int>(colourVec.r * 255), 0, 255),
		std::clamp(static_cast<int>(colourVec.g * 255), 0, 255),
		std::clamp(static_cast<int>(colourVec.b * 255), 0, 255)
	);
}

// Single light shadow check
bool RayTracer::isInShadow(const glm::vec3& pos, const PointLight& light, const std::vector<ModelTriangle>& triangles) {
	glm::vec3 lightDir = glm::normalize(light.position - pos);
	glm::vec3 shadowOrigin = pos + lightDir * 0.001f;
	Ray shadowRay(shadowOrigin, lightDir);

	float lightDistance = glm::length(light.position - pos);
	for (const auto& triangle : triangles) {
		float t;
		if (findIntersection(triangle, shadowRay, t) && t < lightDistance) {
			// Check if the triangle is facing the light
			if (glm::dot(triangle.normal, shadowRay.direction) < 0.0f) {
				return true;
			}
		}
	}
	return false;
}

glm::vec3 computeInterpolatedNormal(const ModelTriangle& triangle, const glm::vec3& intersectionPoint) {
	glm::vec3 v0 = triangle.vertices[0];
	glm::vec3 v1 = triangle.vertices[1];
	glm::vec3 v2 = triangle.vertices[2];

	glm::vec3 k = convertToBarycentricCoordinates(v0, v1, v2, intersectionPoint);

	glm::vec3 interpolatedNormal =
		glm::normalize(k.x * triangle.vertexNormals[0] +
			k.y * triangle.vertexNormals[1] +
			k.z * triangle.vertexNormals[2]);

	return interpolatedNormal;
}

Colour getTextureColour(const std::unordered_map<std::string, TextureMap>& allTextureMaps, const ModelTriangle& hitTriangle, glm::vec3& intersectionPoint) {
	const std::string mapName = hitTriangle.material.mapName;
	auto iterator = allTextureMaps.find(mapName);
	TextureMap textureMap;;
	if (iterator != allTextureMaps.end()) {
		textureMap = iterator->second;
	}
	else {
		return hitTriangle.material.Kd;
	}

	// Texture mapping with given Triangle and intersection point
	glm::vec3 v0 = hitTriangle.vertices[0];
	glm::vec3 v1 = hitTriangle.vertices[1];
	glm::vec3 v2 = hitTriangle.vertices[2];

	float alpha, beta, gamma;
	glm::vec3 k = convertToBarycentricCoordinates(v0, v1, v2, intersectionPoint);
	float u = k.x * hitTriangle.u.x +
			  k.y * hitTriangle.u.y +
		      k.z * hitTriangle.u.z;
	float v = k.x * hitTriangle.v.x +
		      k.y * hitTriangle.v.y +
		      k.z * hitTriangle.v.z;

	// No need to wrap here
	int textureX = int(u * textureMap.width) % textureMap.width;
	int textureY = int((1 - v) * textureMap.height) % textureMap.height;
	uint32_t baseColourInt = textureMap.pixels[textureX + textureY * textureMap.width];
	Colour textureColour(
		(baseColourInt >> 16) & 0xFF,
		(baseColourInt >> 8) & 0xFF,
		baseColourInt & 0xFF
	);
	return textureColour;
}

glm::vec3 RayTracer::traceRay(
	const Ray& ray,
	const std::vector<ModelTriangle>& triangles,
	const std::unordered_map<std::string, TextureMap>& allTextureMaps,
	int depth
) {
	// Base case
	if (depth < 0) {
		return glm::vec3(0.0f);
	}

	ModelTriangle hitTriangle;
	glm::vec3 intersectionPoint;
	float t;
	Colour shaded(0, 0, 0);
	// Find nearest intersection
	if (findNearestIntersection(ray, triangles, hitTriangle, intersectionPoint, t)) {
		glm::vec3 normal = computeInterpolatedNormal(hitTriangle, intersectionPoint);
		// Solid colour
		if (hitTriangle.material.type == "kd") {
			Colour& baseColour = hitTriangle.material.Kd;
			shaded = computePhongLighting(intersectionPoint, triangles, normal, baseColour);
		}
		// Texture mapping
		else if (hitTriangle.material.type == "kd_map") {
			Colour textureColour = getTextureColour(allTextureMaps, hitTriangle, intersectionPoint);
			shaded = computePhongLighting(intersectionPoint, triangles, normal, textureColour);
		}
		// Reflection
		else if (hitTriangle.material.type == "reflect") {
			glm::vec3 reflectDir = glm::reflect(ray.direction, normal);
			Ray reflectRay(intersectionPoint + reflectDir * 0.01f, reflectDir);

			ModelTriangle reflectHitTriangle;
			glm::vec3 reflectIntersectionPoint;
			float reflectT;

			Colour reflectColour = vec3ToColour(traceRay(reflectRay, triangles, allTextureMaps, depth - 1));
			Colour baseHitColour = hitTriangle.material.Kd;

			Colour localColour = computePhongLighting(
				intersectionPoint,
				triangles,
				normal,
				baseHitColour
			);
			float reflectStrength = hitTriangle.material.reflectStrength;
			shaded = localColour * (1.0f - reflectStrength) + reflectColour * reflectStrength;
		}
		// Metallic
		else if (hitTriangle.material.type == "metallic") {
			glm::vec3 reflectDir = glm::reflect(ray.direction, normal);
			Ray reflectRay(intersectionPoint + reflectDir * 0.01f, reflectDir);
			// Add metallic in lighting
			float metallic = hitTriangle.material.metallic;
			Colour metalColour = hitTriangle.material.metalColour;
			Colour localColour = computePhongLighting(intersectionPoint, triangles, normal, metalColour, metallic);

			Colour reflectColour = vec3ToColour(traceRay(reflectRay, triangles, allTextureMaps, depth - 1));
			float reflectStrength = hitTriangle.material.reflectStrength;
			shaded = localColour * (1.0f - reflectStrength) + reflectColour * reflectStrength;
		}
		// Glass
		else if (hitTriangle.material.type == "glass") {
			float ior = hitTriangle.material.refractFactor;
			float transparency = hitTriangle.material.transparency;

			glm::vec3 I = glm::normalize(ray.direction);
			glm::vec3 N = glm::normalize(normal);

			float n1 = 1.0f, n2 = ior;
			float cosi = glm::dot(I, N);

			if (cosi > 0.0f) {
				N = -N;
				std::swap(n1, n2);
				cosi = glm::dot(I, N);
			}
			float eta = n1 / n2;
			glm::vec3 refractDir = glm::refract(I, N, eta);
			glm::vec3 reflectDir = glm::normalize(glm::reflect(I, N));
			bool tir = (glm::length(refractDir) < 1e-6f);
			// Fresnel-Schlick to get reflection coefficient
			float R0 = (n1 - n2) / (n1 + n2);
			R0 = R0 * R0;
			float cosTheta = glm::clamp(-cosi, 0.0f, 1.0f);
			float R = R0 + (1.0f - R0) * pow(1.0f - cosTheta, 5.0f);
			
			glm::vec3 refractVec(0.0f);
			glm::vec3 reflectVec(0.0f);

			if (!tir) {
				refractVec = traceRay(Ray(intersectionPoint + refractDir * 0.01f, glm::normalize(refractDir)), triangles, allTextureMaps, depth - 1);
			}
			reflectVec = traceRay(Ray(intersectionPoint + reflectDir * 0.01f, reflectDir), triangles, allTextureMaps, depth - 1);
			// Combine
			glm::vec3 totalVec = tir ? reflectVec
				: (R * reflectVec + (1.0f - R) * transparency * refractVec);
			totalVec = glm::clamp(totalVec, 0.0f, 1.0f);
			shaded = Colour(int(totalVec.r * 255), int(totalVec.g * 255), int(totalVec.b * 255));
		}
	}
	// Env Mapping
	else {
		if (!useEnvMap || depth == 3) {
			return glm::vec3(0.0f);
		}
		glm::vec3 dir = glm::normalize(ray.direction);
		glm::vec2 uv = directionToUV(dir);
		int textureX = int(uv.x * envMap.width) % envMap.width;
		int textureY = int((1 - uv.y) * envMap.height) % envMap.height;
		int index = int(textureY * envMap.width) + textureX;
		int32_t colourInt = envMap.pixels[index];
		shaded = Colour(
			(colourInt >> 16) & 0xFF,
			(colourInt >> 8) & 0xFF,
			colourInt & 0xFF
		);
	}
	return glm::vec3(shaded.red, shaded.green, shaded.blue) / 255.0f;
}

void RayTracer::RenderRows(
	int yStart, int yEnd, 
	const std::vector<ModelTriangle>& triangles, 
	const std::unordered_map<std::string, TextureMap>& allTextureMaps,
	std::vector<uint32_t>& colourBuffer
) {
	// prevent same random sequence in each thread
	std::mt19937 rng(1234u + (unsigned)yStart * 5678u);
	for (int y = yStart; y < yEnd; y++) {
		for (int x = 0; x < width; x++) {
			// Transformation
			float xNDC = (2.0f * (x + 0.5f) / width - 1.0f);
			float yNDC = (1.0f - 2.0f * (y + 0.5f) / height);
			glm::vec4 nearPointClip(xNDC, yNDC, -1.0f, 1.0f);
			glm::vec4 farPointClip(xNDC, yNDC, 1.0f, 1.0f);
			glm::vec4 nearWorld = invPV * nearPointClip;
			glm::vec4 farWorld = invPV * farPointClip;
			nearWorld /= nearWorld.w;
			farWorld /= farWorld.w;

			// Get Main Ray
			glm::vec3 rayOrigin = glm::vec3(nearWorld);
			glm::vec3 rayDir = glm::normalize(glm::vec3(farWorld - nearWorld));

			if (lensRadius <= 0.0f) {
				// No Depth of Field
				glm::vec3 colour = traceRay(Ray(rayOrigin, rayDir), triangles, allTextureMaps, 3);
				colour = glm::clamp(colour, 0.0f, 1.0f);
				uint8_t r = (uint8_t)(colour.r * 255.0f);
				uint8_t g = (uint8_t)(colour.g * 255.0f);
				uint8_t b = (uint8_t)(colour.b * 255.0f);
				colourBuffer[x + y * width] = (255u << 24) + (r << 16) + (g << 8) + b;
				continue;
			}
			else {
				// Depth of Field
				glm::vec3 O = camera;
				glm::vec3 D = rayDir;
				glm::vec3 p0 = camera + camForward * focusDist;

				float denom = glm::dot(D, camForward);
				// Prevent division by zero
				float tFocus = (fabs(denom) < 1e-6f) ? focusDist : glm::dot(p0 - O, camForward) / denom;
				glm::vec3 focusPoint = O + tFocus * D;
				glm::vec3 sum = glm::vec3(0.0f);
				int spp = sppDOF;
				for (int s = 0; s < spp; ++s) {
					glm::vec2 d = randomInUnitDisk(rng) * lensRadius;
					glm::vec3 origin = camera + camRight * d.x + camUp * d.y;
					glm::vec3 dir = glm::normalize(focusPoint - origin);
					sum += traceRay(Ray(origin, dir), triangles, allTextureMaps, 3);
				}
				// Average
				sum /= float(spp);
				sum = glm::clamp(sum, 0.0f, 1.0f);
				uint8_t r = (uint8_t)(sum.r * 255.0f);
				uint8_t g = (uint8_t)(sum.g * 255.0f);
				uint8_t b = (uint8_t)(sum.b * 255.0f);
				colourBuffer[x + y * width] = (255u << 24) + (r << 16) + (g << 8) + b;
			}
		}
	}
}

void RayTracer::drawAllTriangles(
	DrawingWindow& window,
	const std::unordered_map<std::string, TextureMap>& allTextureMaps,
	const std::vector<ModelTriangle>& triangles
) {
	// Compute buffer not directly on window
	std::vector<uint32_t> colourBuffer(width * height, 0);

	unsigned int numThreads = 16;
	//std::cout << "Number of threads: " << numThreads << std::endl;
	
	// Launch threads to render rows
	std::vector<std::thread> threads;
	threads.reserve(numThreads);

	int rowsPerThread = (height + (int)numThreads - 1) / (int)numThreads;
	for (unsigned int i = 0; i < numThreads; ++i) {
		int y0 = (int)i * rowsPerThread;
		int y1 = std::min(height, y0 + rowsPerThread);
		threads.emplace_back(
			&RayTracer::RenderRows, this,
			y0, y1,
			std::cref(triangles),
			std::cref(allTextureMaps),
			std::ref(colourBuffer)
		);
	}
	for (auto& th : threads) th.join();

	// Write to window
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = y * width + x;
			window.setPixelColour(x, y, colourBuffer[idx]);
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