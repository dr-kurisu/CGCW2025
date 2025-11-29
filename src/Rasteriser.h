#include "ModelTriangle.h"
#include "DrawingWindow.h"
#include "FileLoader.h"
#include <algorithm>
#include <unordered_map>
#include "Renderer.h"

class Rasteriser : public Renderer{
public:
	// Pass width and height for coordinate transformation
	Rasteriser(int w, int h);
	~Rasteriser();

	void drawWireframeTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour);

	void drawFilledTriangle(DrawingWindow& window, ModelTriangle triangle);

	void drawTextureTriangle(DrawingWindow& window, ModelTriangle t, Buffers &buffers);

	void drawAllTriangles(DrawingWindow& window, Buffers &buffers, const std::vector<ModelTriangle>& triangles) override;

	void updateMatrices(glm::mat4& viewMat, glm::mat4& projMat) override {};

	void resetZBuffer() override;

private:
	std::vector<float> zbuffer;
	int width;
	int height;
	std::string currentTextureName = "";
};