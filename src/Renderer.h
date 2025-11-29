#ifndef RENDERER_H
#define RENDERER_H

#include "DrawingWindow.h"
#include "FileLoader.h"

class Renderer {
public:
    virtual void resetZBuffer() = 0;
	virtual void updateMatrices(glm::mat4& viewMat, glm::mat4& projMat) {};
	virtual void drawAllTriangles(DrawingWindow& window, Buffers& buffers, const std::vector<ModelTriangle>& triangles) = 0;
    virtual ~Renderer() = default;
};

#endif