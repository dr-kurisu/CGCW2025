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

// Todo: Add Z-buffer

struct Line {
    std::vector<glm::vec2> points;
};

inline float edgeFunction(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

auto wrap = [](float x) {
    return x - std::floor(x);
    };

void Rasteriser::resetZBuffer() {
    std::fill(zbuffer.begin(), zbuffer.end(), -std::numeric_limits<float>::infinity());
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

void Rasteriser::drawFilledTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour) {
    // Find bounding box
    auto vertices = triangle.vertices;
    int minX = std::floor(std::min({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int maxX = std::ceil(std::max({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int minY = std::floor(std::min({ vertices[0].y, vertices[1].y, vertices[2].y }));
    int maxY = std::ceil(std::max({ vertices[0].y, vertices[1].y, vertices[2].y }));
    float area = edgeFunction(vertices[0], vertices[1], vertices[2]);
    // Invalid triangle
    if (area == 0) return;
    for (int y = minY; y < maxY; y++) {
        if (y < 0 || y > window.height - 1) continue;
        for (int x = minX; x < maxX; x++) {
            if (x < 0 || x > window.width - 1) continue; // out of boundary
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
                uint32_t colourInt = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
                window.setPixelColour(x, y, colourInt);
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

void Rasteriser::DrawTextureTriangle(DrawingWindow& window, ModelTriangle t) {
    // Find bounding box
    auto vertices = t.vertices;
    int minX = std::floor(std::min({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int maxX = std::ceil(std::max({ vertices[0].x, vertices[1].x, vertices[2].x }));
    int minY = std::floor(std::min({ vertices[0].y, vertices[1].y, vertices[2].y }));
    int maxY = std::ceil(std::max({ vertices[0].y, vertices[1].y, vertices[2].y }));

    float area = edgeFunction(vertices[0], vertices[1], vertices[2]);
    // Invalid triangle
    if (area == 0) return;

    assert(t.hasTexture());

    // Find textureMap
    const std::string mapName = t.material.mapName;
    auto iterator = textureMaps.find(mapName);
    if (iterator != textureMaps.end()) {
        textureMap = iterator->second;
    }
    else {
        std::cerr << "Texture not found: " << mapName << std::endl;
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

                int zBufferIndex = y * window.width + x;

                if (z <= zbuffer[zBufferIndex]) {
                    continue;
                }
                zbuffer[zBufferIndex] = z;

				// should be between 0 and 1
                glm::vec3 k = glm::vec3(alpha, beta, gamma);
				float one_over_w = glm::dot(k, t.inverseW);

                float u = glm::dot(k, t.u * t.inverseW);
                float v = glm::dot(k, t.v * t.inverseW);
				// perspective correct something wrong
				u /= one_over_w;
				v /= one_over_w;

                //float u = alpha * t.texturePoints[0].x + 
                //          beta * t.texturePoints[1].x +
                //          gamma * t.texturePoints[2].x;
                //float v = alpha * t.texturePoints[0].y +
                //          beta * t.texturePoints[1].y +
                //          gamma * t.texturePoints[2].y;

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

// May need to rename
void Rasteriser::DrawAllTriangles(DrawingWindow& window, const std::vector<ModelTriangle>& triangles) {
    window.clearPixels();
    for (auto t : triangles) {
        // drawWireframeTriangle(window, t, WHITE);
        DrawTextureTriangle(window, t);
    }
}

void Rasteriser::loadTexture(const std::string& name, const TextureMap& texture) {
    textureMaps[name] = texture;
}

// constructor
Rasteriser::Rasteriser(int w, int h)
    : width(w),
    height(h),
    zbuffer(w* h, -std::numeric_limits<float>::infinity()),
    textureMaps()
{}

Rasteriser::~Rasteriser() {
}

