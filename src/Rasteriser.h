#include "ModelTriangle.h"
#include "DrawingWindow.h"
#include "FileLoader.h"
#include "PointLight.h"
#include <algorithm>
#include <unordered_map>
#include "Renderer.h"

enum class ShadeMode { Flat, Gouraud, Phong, Wireframe };

class Rasteriser {
public:
	// Pass width and height for coordinate transformation
	Rasteriser(int w, int h);
	~Rasteriser();

	Colour computePhongLighting(
		const glm::vec3& pos,
		const glm::vec3& normal,
		const Colour& Colour
	);

	void drawWireframeTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour);

	void drawFilledTriangle(DrawingWindow& window, ModelTriangle triangle, Colour colour);

	void drawTextureTriangle(DrawingWindow& window, ModelTriangle t, Buffers &buffers);

	void drawPhongTriangle(
		DrawingWindow& window,
		const ModelTriangle& projectedTriangle,
		const ModelTriangle& worldTriangle,
		const glm::vec3& cameraPos,
		const std::vector<PointLight>& lights
	);

	void drawAllTriangles(
		DrawingWindow& window,
		Buffers& buffers,
		const std::vector<ModelTriangle>& projectedTriangles,
		const std::vector<ModelTriangle>& worldTriangles,
		ShadeMode shadeMode,
		bool lighting
	);

	void drawGouraudTriangle(
		DrawingWindow& window,
		const ModelTriangle& triangle,
		const std::array<Colour, 3>& vertexColours
	);

	Colour computeFlatShading(
		const ModelTriangle& triangle,
		const glm::vec3& camera,
		const std::vector<PointLight>& lights
	);

	std::array<Colour, 3> computeGouraudShading(
		const ModelTriangle& triangle,
		const glm::vec3& cameraPos,
		const std::vector<PointLight>& lights
	);

	void updateMatrices(glm::mat4& viewMat, glm::mat4& projMat);

	void resetZBuffer();

	void updateLights(std::vector<PointLight>& inLights);

	void updateCamera(const glm::vec3& inCamera) { camera = inCamera; }

private:
	std::vector<float> zbuffer;
	std::vector<PointLight> lights;
	glm::vec3 camera;
	int width;
	int height;
	float ambient = 0.2f;
	std::string currentTextureName = "";
};