#include "ModelTriangle.h"
#include "DrawingWindow.h"
#include <algorithm>

void drawWireframeTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour);

void DrawTextureTriangle(DrawingWindow& window, ModelTriangle t);

void DrawAllTriangles(DrawingWindow& window, const std::vector<ModelTriangle>& triangles);