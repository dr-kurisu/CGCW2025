#include "ModelTriangle.h"
#include "DrawingWindow.h"
#include <algorithm>
#include <unordered_map>


class Rasteriser {
public:
	// Pass width and height for coordinate transformation
	Rasteriser(int w, int h);
	~Rasteriser();

	void drawWireframeTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour);

	void drawFilledTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour);

	void DrawTextureTriangle(DrawingWindow& window, ModelTriangle t);

	void DrawAllTriangles(DrawingWindow& window, const std::vector<ModelTriangle>& triangles);

	void loadTexture(const std::string& name, const TextureMap& texture);

	void resetZBuffer();

private:
	std::vector<float> zbuffer;
	std::unordered_map<std::string, TextureMap> textureMaps;
	int width;
	int height;

	TextureMap textureMap;
	std::string currentTextureName = "";
};