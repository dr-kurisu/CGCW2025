#include "ModelTriangle.h"
#include "DrawingWindow.h"
#include "Utils.h"
#include "Rasteriser.h"
#include <algorithm>
#include <unordered_map>

#ifndef WIDTH
#define WIDTH 640
#endif

#ifndef HEIGHT
#define HEIGHT 480
#endif

#define WHITE Colour(255, 255, 255)

struct Line {
    std::vector<glm::vec2> points;
};

inline float edgeFunction(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

inline float edgeFunction(const glm::vec3& a, const glm::vec3& b, const glm::vec2& c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

auto wrap = [](float x) {
    return x - std::floor(x);
};

void Rasteriser::resetZBuffer() {
    std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<float>::infinity());
}

Line InterpolateLine(glm::vec3 p1, glm::vec3 p2) {
	int x1 = std::round(p1.x); 
    int y1 = std::round(p1.y);
    int x2 = std::round(p2.x);
	int y2 = std::round(p2.y);
    //x1 = std::max(0, std::min(static_cast<int>(WIDTH) - 1, x1));
    //y1 = std::max(0, std::min(static_cast<int>(HEIGHT) - 1, y1));
    //x2 = std::max(0, std::min(static_cast<int>(WIDTH) - 1, x2));
    //y2 = std::max(0, std::min(static_cast<int>(HEIGHT) - 1, y2));


    Line line;
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int numSteps = std::max(dx, dy);
    float stepX = dx / float(numSteps) * ((x2 - x1) < 0 ? -1 : 1);
    float stepY = dy / float(numSteps) * ((y2 - y1) < 0 ? -1 : 1);
    for (float i = 0.0; i < numSteps; i++) {
        float x = x1 + i * stepX;
        float y = y1 + i * stepY;
		// Get rid of out of boundary points
        if (x < 0 || y < 0 || x > WIDTH - 1 || y > HEIGHT - 1) continue;
        line.points.push_back(glm::vec2(x, y));
    }
    return line;
}

void drawLineWithColour(DrawingWindow& window, Line line, Colour c) {
    for (size_t i = 0; i < line.points.size(); i++) {
        int x = round(line.points[i].x);
        int y = round(line.points[i].y);
		if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) continue; // out of boundary
        uint32_t colour = (255 << 24) + (c.red << 16) + (c.green << 8) + c.blue;
        window.setPixelColour(x, y, colour);
    }
}

Colour Rasteriser::computePhongLighting(
    const glm::vec3& pos,
    const glm::vec3& normal,
    const Colour& colour
) {
    // No shadow
    glm::vec3 base = glm::vec3(colour.red, colour.green, colour.blue) / 255.0f;
    glm::vec3 ambientv3 = glm::vec3(ambient) * base;
    glm::vec3 total = ambientv3;
    const float EPSILON = 1e-6f;

    for (const auto& light : lights) {
        glm::vec3 lightDir = glm::normalize(light.position - pos);
        glm::vec3 viewDir = glm::normalize(camera - pos);
        glm::vec3 halfDir = glm::normalize(lightDir + viewDir);

        // Diffuse
        float diff = std::max(0.0f, glm::dot(normal, lightDir));

        // Specular
        float spec = pow(std::max(0.0f, glm::dot(normal, halfDir)), shininess);

        float d = glm::length(light.position - pos);
		if (d < EPSILON) d = EPSILON;
        float attenuation = 250.0f / (4 * M_PI * d * d);

        glm::vec3 lightCol = light.colour * light.intensity * attenuation;
        glm::vec3 diffuse = diff * lightCol * base;
        glm::vec3 specular = specStrength * spec * lightCol;

        total += diffuse + specular;
    }

    total = glm::clamp(total, 0.0f, 1.0f);
    return Colour(int(total.r * 255), int(total.g * 255), int(total.b * 255));
}

Colour Rasteriser::computeFlatShading(
    const ModelTriangle& triangle,
    const glm::vec3& camera,
    const std::vector<PointLight>& lights
) {
    // All in world position
    glm::vec3 v0 = triangle.vertices[0];
    glm::vec3 v1 = triangle.vertices[1];
    glm::vec3 v2 = triangle.vertices[2];
    // sample centre point
    glm::vec3 centroid = (v0 + v1 + v2) / 3.0f;
	glm::vec3 normal = triangle.normal;

    Colour base;
    if (triangle.material.type == "kd") {
        base = triangle.material.Kd;
    }
    else {
		// Deal with texture
		base = WHITE; // Default white
    }
    return computePhongLighting(
        centroid,
        normal,
        triangle.material.Kd
    );
}

std::array<Colour, 3> Rasteriser::computeGouraudShading(
    const ModelTriangle& triangle,
    const glm::vec3& cameraPos,
    const std::vector<PointLight>& lights
) {
    std::array<Colour, 3> vertexColours;
    for (int i = 0; i < 3; i++) {
        glm::vec3 vertexPos = triangle.vertices[i];
        glm::vec3 normal = glm::normalize(triangle.vertexNormals[i]);

        vertexColours[i] = computePhongLighting(
            vertexPos,
            normal,
            triangle.material.Kd
        );
    }
    return vertexColours;
}

void Rasteriser::drawGouraudTriangle(
    DrawingWindow& window,
    const ModelTriangle& triangle,
    const std::array<Colour, 3>& vertexColours
) {
    auto vertices = triangle.vertices;
    int minX = std::floor(std::min({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int maxX = std::ceil(std::max({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int minY = std::floor(std::min({ vertices[0].y, vertices[1].y, vertices[2].y }));
    int maxY = std::ceil(std::max({ vertices[0].y, vertices[1].y, vertices[2].y }));

    float area = edgeFunction(vertices[0], vertices[1], vertices[2]);
    if (abs(area) < 1e-6) return; // invalid

    for (int y = minY; y < maxY; y++) {
        if (y < 0 || y >= window.height) continue;

        for (int x = minX; x < maxX; x++) {
            if (x < 0 || x >= window.width) continue;

            glm::vec2 p = glm::vec2(x + 0.5f, y + 0.5f);
            float w0 = edgeFunction(vertices[1], vertices[2], p);
            float w1 = edgeFunction(vertices[2], vertices[0], p);
            float w2 = edgeFunction(vertices[0], vertices[1], p);

            if (area < 0) {
                w0 = -w0; w1 = -w1; w2 = -w2;
            }

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float alpha = w0 / area;
                float beta = w1 / area;
                float gamma = w2 / area;

                float inverseW = alpha * triangle.inverseW[0] +
                    beta * triangle.inverseW[1] +
                    gamma * triangle.inverseW[2];

                float z = alpha * vertices[0].z +
                    beta * vertices[1].z +
                    gamma * vertices[2].z;

                float zNdc = z / inverseW;
                int zBufferIndex = y * window.width + x;

                // Colour interpolation
                if (zNdc < zbuffer[zBufferIndex]) {
                    zbuffer[zBufferIndex] = zNdc;
                    int r = int(alpha * vertexColours[0].red +
                        beta * vertexColours[1].red +
                        gamma * vertexColours[2].red);
                    int g = int(alpha * vertexColours[0].green +
                        beta * vertexColours[1].green +
                        gamma * vertexColours[2].green);
                    int b = int(alpha * vertexColours[0].blue +
                        beta * vertexColours[1].blue +
                        gamma * vertexColours[2].blue);

                    uint32_t colourInt = (255 << 24) + (r << 16) + (g << 8) + b;
                    window.setPixelColour(x, y, colourInt);
                }
            }
        }
    }
}

void Rasteriser::drawPhongTriangle(
    DrawingWindow& window,
    const ModelTriangle& projectT,
    const ModelTriangle& worldT,
    const glm::vec3& cameraPos,
    const std::vector<PointLight>& lights
) {
    auto vertices = projectT.vertices;
    int minX = std::floor(std::min({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int maxX = std::ceil(std::max({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int minY = std::floor(std::min({ vertices[0].y, vertices[1].y, vertices[2].y }));
    int maxY = std::ceil(std::max({ vertices[0].y, vertices[1].y, vertices[2].y }));

    double area = edgeFunction(vertices[0], vertices[1], vertices[2]);
    if (abs(area) < 1e-6) return; // invalid triangle

    for (int y = minY; y < maxY; y++) {
        if (y < 0 || y >= window.height) continue;

        for (int x = minX; x < maxX; x++) {
            if (x < 0 || x >= window.width) continue;

            glm::vec2 p = glm::vec2(x + 0.5f, y + 0.5f);
            float w0 = edgeFunction(vertices[1], vertices[2], p);
            float w1 = edgeFunction(vertices[2], vertices[0], p);
            float w2 = edgeFunction(vertices[0], vertices[1], p);

            if (area < 0) { w0 = -w0; w1 = -w1; w2 = -w2; }

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float alpha = w0 / area;
                float beta = w1 / area;
                float gamma = w2 / area;
                // Projection correctness
                float invW = alpha * projectT.inverseW[0] +
                      beta * projectT.inverseW[1] +
                      gamma * projectT.inverseW[2];
                float z = alpha * vertices[0].z +
                    beta * vertices[1].z +
                    gamma * vertices[2].z;
                float zNdc = z / invW;
                int zIndex = y * window.width + x;

                if (zNdc < zbuffer[zIndex]) {
                    zbuffer[zIndex] = zNdc;

                    // Normals interpolation
                    glm::vec3 n0 = glm::normalize(worldT.vertexNormals[0]);
                    glm::vec3 n1 = glm::normalize(worldT.vertexNormals[1]);
                    glm::vec3 n2 = glm::normalize(worldT.vertexNormals[2]);
                    glm::vec3 interpNormal = glm::normalize(
                        alpha * n0 + beta * n1 + gamma * n2
                    );
                    // World Coordinate interpolation
                    glm::vec3 v0 = worldT.vertices[0];
                    glm::vec3 v1 = worldT.vertices[1];
                    glm::vec3 v2 = worldT.vertices[2];
                    glm::vec3 interpPos =
                        alpha * v0 + beta * v1 + gamma * v2;

                    // Compute Colour
                    Colour pixelColour = computePhongLighting(interpPos, interpNormal, worldT.material.Kd);

                    uint32_t colourInt =
                        (255 << 24) + (pixelColour.red << 16) +
                        (pixelColour.green << 8) + pixelColour.blue;

                    window.setPixelColour(x, y, colourInt);
                }
            }
        }
    }
}

void Rasteriser::drawFilledTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour) {
    // Find bounding box
    auto vertices = triangle.vertices;
    int minX = std::floor(std::min({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int maxX = std::ceil(std::max({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int minY = std::floor(std::min({ vertices[0].y, vertices[1].y, vertices[2].y }));
    int maxY = std::ceil(std::max({ vertices[0].y, vertices[1].y, vertices[2].y }));
    double area = edgeFunction(vertices[0], vertices[1], vertices[2]);
    // Invalid triangle
    if (abs(area) < 1e-6) return; 
    for (int y = minY; y < maxY; y++) {
        if (y < 0 || y > window.height - 1) continue;
        for (int x = minX; x < maxX; x++) {
            if (x < 0 || x > window.width - 1) continue; // out of boundary
            glm::vec2 p = glm::vec2(x + 0.5f, y + 0.5f); // sample center of pixel
            float w0 = edgeFunction(vertices[1], vertices[2], p);
            float w1 = edgeFunction(vertices[2], vertices[0], p);
            float w2 = edgeFunction(vertices[0], vertices[1], p);
            // check order of vertices
            if (area < 0) {
                w0 = -w0;
                w1 = -w1;
                w2 = -w2;
            }
            // interpolate colour
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float alpha = w0 / area;
                float beta = w1 / area;
                float gamma = w2 / area;

                float inverseW = alpha * triangle.inverseW[0] +
                             beta * triangle.inverseW[1] +
                             gamma * triangle.inverseW[2];

                float z = alpha * vertices[0].z +
                          beta * vertices[1].z +
                          gamma * vertices[2].z;

                float zNdc = z / inverseW;

                int zBufferIndex = y * window.width + x;

                if (zNdc < zbuffer[zBufferIndex]) {
                    zbuffer[zBufferIndex] = zNdc;
                    uint32_t colourInt = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
                    window.setPixelColour(x, y, colourInt);
                }
            }            
        }
    }
}

void Rasteriser::drawWireframeTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour) {
    glm::vec3 v1 = triangle.vertices[0];
    glm::vec3 v2 = triangle.vertices[1];
    glm::vec3 v3 = triangle.vertices[2];

    Line line1 = InterpolateLine(v1, v2);
    Line line2 = InterpolateLine(v2, v3);
    Line line3 = InterpolateLine(v3, v1);
    drawLineWithColour(window, line1, colour);
    drawLineWithColour(window, line2, colour);
    drawLineWithColour(window, line3, colour);
}

void Rasteriser::drawTextureTriangle(DrawingWindow& window, ModelTriangle t, Buffers &buffers) {
	// Only draw if triangle has texture
    if (t.material.type != "kd_map") return;
    assert(t.hasTexture());
    // Find bounding box
    auto vertices = t.vertices;
    int minX = std::floor(std::min({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int maxX = std::ceil(std::max({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int minY = std::floor(std::min({ vertices[0].y, vertices[1].y, vertices[2].y }));
    int maxY = std::ceil(std::max({ vertices[0].y, vertices[1].y, vertices[2].y }));

    float area = edgeFunction(vertices[0], vertices[1], vertices[2]);
    // Invalid triangle
    if (area == 0) return;

    // Find textureMap
    const std::string mapName = t.material.mapName;
    auto iterator = buffers.textureMaps.find(mapName);
	TextureMap& textureMap = buffers.textureMaps.begin()->second; // default to first texture
    if (iterator != buffers.textureMaps.end()) {
        // No value copy
        textureMap = iterator->second;
    }
    else {
        std::cerr << "Texture not found: " << mapName << std::endl;
        return;
    }
 
    for (int y = minY; y < maxY; y++) {
        if (y < 0 || y >= window.height) continue; // out of boundary
        for (int x = minX; x < maxX; x++) {
			if (x < 0 || x >= window.width) continue; // out of boundary
			glm::vec2 p = glm::vec2(x + 0.5f, y + 0.5f); // sample center of pixel
            float w0 = edgeFunction(vertices[1], vertices[2], glm::vec3(p, 0.0f));
            float w1 = edgeFunction(vertices[2], vertices[0], glm::vec3(p, 0.0f));
            float w2 = edgeFunction(vertices[0], vertices[1], glm::vec3(p, 0.0f));
			// check order of vertices
            if (area < 0) {
                w0 = -w0;
                w1 = -w1;
                w2 = -w2;
            }
            // interpolate colour
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                float alpha = w0 / area;
                float beta = w1 / area;
                float gamma = w2 / area;

                float z = alpha * t.vertices[0].z +
                          beta * t.vertices[1].z +
                          gamma * t.vertices[2].z;

                float inverseW = alpha * t.inverseW[0] +
                                 beta * t.inverseW[1] +
                                 gamma * t.inverseW[2];

                int zBufferIndex = y * window.width + x;

				float zNdc = z / inverseW;

                if (zNdc >= zbuffer[zBufferIndex]) {
                    continue;
                }
				zbuffer[zBufferIndex] = zNdc;

				// should be between 0 and 1
                glm::vec3 k = glm::vec3(alpha, beta, gamma);
				float one_over_w = glm::dot(k, t.inverseW);

                float u = glm::dot(k, t.u * t.inverseW);
                float v = glm::dot(k, t.v * t.inverseW);
				// perspective correct
				u /= one_over_w;
				v /= one_over_w;

				// Wrap texture coordinates
                float uWrap = u - std::floor(u);
                float vWrap = v - std::floor(v);

                int textureX = int(uWrap * textureMap.width) % textureMap.width;
                int textureY = int((1 - vWrap) * textureMap.height) % textureMap.height;
                uint32_t colourInt = textureMap.pixels[textureX + textureY * textureMap.width];
                window.setPixelColour(x, y, colourInt);
            }            
        }
    }
}

void Rasteriser::drawAllTriangles(
    DrawingWindow& window,
    Buffers& buffers,
    const std::vector<ModelTriangle>& projectedTriangles,
    const std::vector<ModelTriangle>& worldTriangles,
    ShadeMode shadeMode,
    bool lighting
) {
    for (size_t i = 0; i < projectedTriangles.size(); i++) {
        const ModelTriangle& projectedT = projectedTriangles[i];
        const ModelTriangle& worldT = worldTriangles[i];
		// Animation Wireframe mode
        if (useWireframe) {
            drawWireframeTriangle(window, projectedT, WHITE);
            continue;
		}

        // Material KD COLOUR
        if (worldT.material.type == "kd" || worldT.material.type == "reflect" || worldT.material.type == "metallic" || worldT.material.type == "glass") {
            Colour colour;
            // without Light
            if (!lighting || useLighting) {
                drawFilledTriangle(window, projectedT, worldT.material.Kd);
                continue;
            }
            // with light
            switch (shadeMode) {
                case ShadeMode::Flat: {
                    colour = computeFlatShading(worldT, camera, lights);
                    drawFilledTriangle(window, projectedT, colour);
                    break;
                }
                case ShadeMode::Gouraud: {
                    std::array<Colour, 3> vertexColours = computeGouraudShading(worldT, camera, lights);
                    drawGouraudTriangle(window, projectedT, vertexColours);
                    break;
                }
                case ShadeMode::Phong: {
                    // Phong shading
                    drawPhongTriangle(window, projectedT, worldT, camera, lights);
                    break;
                }
                case ShadeMode::Wireframe: {
                    drawWireframeTriangle(window, projectedT, WHITE);
                    break;
                }
            }            
        }
		// Material KD MAP temp no lighting
		else if (useTextures && worldT.material.type == "kd_map") {
            drawTextureTriangle(window, projectedT, buffers); 
        }
		else { 
            drawWireframeTriangle(window, projectedT, WHITE); 
        }
    }
}

void Rasteriser::updateLights(std::vector<PointLight>& inLights) {
	lights = inLights;
}


Rasteriser::Rasteriser(int w, int h)
    : width(w),
    height(h),
    zbuffer(w* h, std::numeric_limits<float>::infinity())
{}

Rasteriser::~Rasteriser() {
}

