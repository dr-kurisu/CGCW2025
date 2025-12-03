#include <iostream>
#include <string>
#include "Transform.h"
#include "Utils.h"
#include "Rasteriser.h"
#include "RayTracer.h"
#include "FileLoader.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

#define WIDTH 640
#define HEIGHT 480

// Key Parameters
bool video = true;
// const string renderMode = "RayTracer";
const string renderMode = "Rasteriser";

SDL_Event event;
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
// Define View Matrix
glm::vec3 camera(0, 0, 430);
glm::vec3 center(0, 0, 0);
glm::vec3 cameraUp(0, 1, 0);
glm::vec3 cameraRight(1, 0, 0);
glm::vec3 cameraForward(0, 0, 1);

glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);

// Define Projection Matrix
float fov = 60.0f;
float aspect = (float)window.width / (float)window.height;
float near = 0.1f;
float far = 1000.0f; 
glm::mat4 projMat = glm::perspective(glm::radians(fov), aspect, near, far);


Buffers logoBuffers;
vector<ModelTriangle> logo;
vector<ModelTriangle> transformedLogo;
vector<ModelTriangle> projectedLogo;

Buffers cornellBuffers;
vector<ModelTriangle> cornell;
vector<ModelTriangle> transformedCornell;
vector<ModelTriangle> projectedcornell;

Buffers sphereBuffers;
vector<ModelTriangle> sphere;
vector<ModelTriangle> transformedSphere;
vector<ModelTriangle> projectedSphere;

mat4 logoModelMat = mat4(1.0f); // Model Matrix for logo
mat4 cornellModelMat = mat4(1.0f); // Model Matrix for cornell
mat4 sphereModelMat = mat4(1.0f); // Model Matrix for sphere

std::vector<ModelTriangle> allTransformedTriangles = {};
std::unordered_map<string, TextureMap> AllTextureMaps = {};
TextureMap envMap = TextureMap("./models/bg.ppm");

vector<PointLight> lights = {
	// PointLight(glm::vec3(0.0f, 20.0f, 50.0f), glm::vec3(1.0f, 1.0f, 1.0f), 500.0f, "FrontLight"),
};

// Define Rasteriser and RayTracer
int frameID = 0;
Rasteriser* rasteriser;
RayTracer* rayTracer;
Transform transformer = Transform(camera, center, cameraUp, cameraRight, cameraForward); // For camera movement

void loadCornell() {
	cornellModelMat = glm::translate(cornellModelMat, vec3(0.0f, 0.0f, 0.0f));
	cornellBuffers = readOBJFile("./models/cornell-box.obj", 50.0f);
	// Load all texture maps
	for (auto& [name, tex] : cornellBuffers.textureMaps) {
		AllTextureMaps[name] = tex;
	}
	// printBuffers(cornellBuffers);
	cornell = convertToTriangles(cornellBuffers); // world coords
	computeNormals(cornellBuffers, cornell);
	transformedCornell = transformTriangles(cornell, cornellModelMat);
	projectedcornell = projectTriangles(transformedCornell, viewMat, projMat);  // screen coords
	if (renderMode == "Rasteriser") {
		rasteriser->drawAllTriangles(window, cornellBuffers, projectedcornell, transformedCornell, ShadeMode::Flat, true);
	}
	else {
		rayTracer->drawAllTriangles(window, AllTextureMaps, transformedCornell);
	}
}

void loadLogo() {
	logoModelMat = glm::translate(logoModelMat, vec3(-250.0f, -250.0f, 0.0f));
	logoBuffers = readOBJFile("./models/logo.obj", 0.5f);
	// Load all texture maps
	for (auto& [name, tex] : logoBuffers.textureMaps) {
		AllTextureMaps[name] = tex;
	}
	/// printBuffers(logoBuffers);
	logo = convertToTriangles(logoBuffers); // world coords
	transformedLogo = transformTriangles(logo, logoModelMat);
	computeNormals(logoBuffers, transformedLogo);
	projectedLogo = projectTriangles(transformedLogo, viewMat, projMat);  // screen coords
	if (renderMode == "Rasteriser") {
		rasteriser->drawAllTriangles(window, logoBuffers, projectedLogo, logo, ShadeMode::Flat, true);
	}
}

void loadSphere() {
	sphereModelMat = glm::translate(sphereModelMat, vec3(-60.0f, -155.0f, 0.0f));
	sphereBuffers = readOBJFile("./models/sphere.obj", 40.0f);
	// Load all texture maps
	for (auto& [name, tex] : sphereBuffers.textureMaps) {
		AllTextureMaps[name] = tex;
	}
	// printBuffers(sphereBuffers);
	sphere = convertToTriangles(sphereBuffers); // world coords
	transformedSphere = transformTriangles(sphere, sphereModelMat);
	computeNormals(sphereBuffers, transformedSphere);
	projectedSphere = projectTriangles(transformedSphere, viewMat, projMat);  // screen coords
	if (renderMode == "Rasteriser") {
		rasteriser->drawAllTriangles(window, sphereBuffers, projectedSphere, transformedSphere, ShadeMode::Phong, true);
	}
}

void update() {
	window.clearPixels();
	viewMat = glm::lookAt(camera, center, cameraUp);
	projectedLogo = projectTriangles(transformedLogo, viewMat, projMat);
	projectedcornell = projectTriangles(transformedCornell, viewMat, projMat);
	projectedSphere = projectTriangles(transformedSphere, viewMat, projMat);
	if (renderMode == "Rasteriser") {
		rasteriser->resetZBuffer();
		rasteriser->updateCamera(camera);
		rasteriser->drawAllTriangles(window, cornellBuffers, projectedcornell, transformedCornell, ShadeMode::Flat, true);
		rasteriser->drawAllTriangles(window, sphereBuffers, projectedSphere, transformedSphere, ShadeMode::Phong, true);
	}
	else {
		rayTracer->updateMatrices(viewMat, projMat);
		rayTracer->updateLights(lights);
		rayTracer->resetZBuffer();
		rayTracer->drawAllTriangles(window, AllTextureMaps, allTransformedTriangles);
	}
}

void handleEvent(SDL_Event event, DrawingWindow& window) {
	if (event.type == SDL_KEYDOWN) {
		// Save Image
		if (event.key.keysym.sym == SDLK_p) {
			cout << "Saving screenshot.bmp" << endl;
			window.saveBMP("./Images/screenshot.bmp");
		}

		// Rotate Camera
		if (event.key.keysym.sym == SDLK_a) {
			transformer.left(5);
			update();
		}
		else if (event.key.keysym.sym == SDLK_d) {
			transformer.left(-5);
			update();
		}
		else if (event.key.keysym.sym == SDLK_w) {
			transformer.up(-5);
			update();
		}
		else if (event.key.keysym.sym == SDLK_s) {
			transformer.up(5);
			update();
		}
		// Translate Camera
		else if (event.key.keysym.sym == SDLK_UP) {
			transformer.moveUp(-10.0f);
			update();
		}
		else if (event.key.keysym.sym == SDLK_DOWN) {
			transformer.moveUp(10.0f);
			update();
		}
		else if (event.key.keysym.sym == SDLK_LEFT) {
			transformer.moveRight(10.0f);
			update();
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			transformer.moveRight(-10.0f);
			update();
		}
	}
	// Dolly Camera
	else if (event.type == SDL_MOUSEWHEEL) {
		if (event.wheel.y > 0) { // scroll up
			transformer.moveForward(30.0f);
		}
		else if (event.wheel.y < 0) { // scroll down
			transformer.moveForward(-30.0f);
		}
		update();
	}
}

void init() {
	// Generate Area Lights
	auto areaLights = generateAreaLight(
		glm::vec3(-20.0f, 80.0f,  1.0f), // leftTop
		glm::vec3(20.0f,  80.0f,  1.0f), // rightTop
		glm::vec3(-22.0f, 80.0f, -1.0f), // leftBottom
		glm::vec3(22.0f,  80.0f, -1.0f), // rightBottom
		10, // sampleX
		2, // sampleZ
		glm::vec3(1.0f),
		1500.0f,
		"TopLight"
	);
	lights.insert(lights.end(), areaLights.begin(), areaLights.end());

	// printLights(lights);

	// Load View and Projection Matrices
	rasteriser = new Rasteriser(window.width, window.height);
	rayTracer = new RayTracer(window.width, window.height);
	// update Matrices
	rayTracer->updateMatrices(viewMat, projMat);
	// reset Z Buffers
	rasteriser->resetZBuffer();
	rayTracer->resetZBuffer();
	// Update Camera and Lights
	rasteriser->updateCamera(camera);
	rayTracer->updateCamera(camera, cameraRight, cameraUp, cameraForward);
	rasteriser->updateLights(lights);
	rayTracer->updateLights(lights);
	rayTracer->loadEnvMap(envMap);
	cout << "Env map loaded" << endl;

	loadCornell();
	loadSphere();

	if (renderMode == "RayTracer") {
		allTransformedTriangles.insert(allTransformedTriangles.end(), transformedCornell.begin(), transformedCornell.end());
		allTransformedTriangles.insert(allTransformedTriangles.end(), transformedSphere.begin(), transformedSphere.end());
		rayTracer->drawAllTriangles(window, AllTextureMaps, allTransformedTriangles);
	}
	std::cout << "Initialisation complete." << std::endl;
}

void updateAnimation(int frameID) {
	if (frameID < 120) {
		cout << "stage 1" << endl;
		rasteriser->useWireframe = true;
		rasteriser->useTextures = false;
		rasteriser->useLighting = false;
		transformer.left(2);
		update();
	}
	else if (frameID < 240) {
		cout << "stage 2" << endl;
		// Add Texture
		rasteriser->useWireframe = false;
		rasteriser->useLighting = false;
		rasteriser->useTextures = true;
		transformer.left(2);
		update();
	}
	else if (frameID < 360) {
		cout << "stage 3" << endl;
		// Add Flat Shading
		rasteriser->useWireframe = false;
		rasteriser->useLighting = true;
		rasteriser->useTextures = true;
		transformer.left(2);
		update();
	}
}

int main(int argc, char* argv[]) {
	init();
	int maxFrames = 360;
	while (true) {
		Uint32 frameStart = SDL_GetTicks();
		if (video) {
			if (frameID >= maxFrames) {
				cout << "Video frames generated." << endl;
				break;
			}
			updateAnimation(frameID);
			window.saveBMP("./Videos01/" + std::to_string(frameID) + ".bmp");
			// window.renderFrame();
			frameID++;
		}
		else {
			if (window.pollForInputEvents(event)) handleEvent(event, window);
			window.renderFrame();
		}
		Uint32 frameTime = SDL_GetTicks() - frameStart;
		if (frameTime < 30) SDL_Delay(30 - frameTime);
	}
	return 0;
}