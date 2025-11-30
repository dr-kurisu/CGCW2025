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
bool video = false;
// const string renderMode = "RayTracer";
const string renderMode = "Rasteriser";

SDL_Event event;
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
// Define View Matrix
glm::vec3 camera(0, 0, 500);
glm::vec3 center(0, 0, 0);
glm::vec3 cameraUp(0, 1, 0);
glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);

// Define Projection Matrix
float fov = 60.0f;
float aspect = (float)window.width / (float)window.height;
float near = 0.5f;
float far = 2000.0f; 
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
mat4 sphereModelMat = mat4(10.f); // Model Matrix for sphere

vector<PointLight> lights = {
	PointLight(glm::vec3(0.0f, 100.0f, 60.0f), glm::vec3(1.0, 1.0, 1.0), 2000.0f, "TopLight"),
	// PointLight(glm::vec3(0.0f, 50.0f, 300.0f), glm::vec3(1.0, 1.0, 1.0), 2000.0f, "FrontLight"),
};

// Define Rasteriser and RayTracer
int frameID = 0;
Rasteriser* rasteriser;
RayTracer* rayTracer;

void loadCornell() {
	cornellModelMat = glm::translate(cornellModelMat, vec3(0.0f, 0.0f, -300.0f));
	cornellBuffers = readOBJFile("./models/cornell-box.obj", 50.0f);
	printBuffers(cornellBuffers);
	cornell = convertToTriangles(cornellBuffers); // world coords
	computeNormals(cornellBuffers, cornell);
	transformedCornell = Transform::transformTriangles(cornell, cornellModelMat);
	projectedcornell = Transform::projectTriangles(transformedCornell, viewMat, projMat);  // screen coords
	if (renderMode == "Rasteriser") {
		rasteriser->drawAllTriangles(window, cornellBuffers, projectedcornell, cornell, ShadeMode::Flat, false);
	}
	else {
		rayTracer->drawAllTriangles(window, cornellBuffers, cornell, camera);
	}
}

void loadLogo() {
	logoModelMat = glm::translate(logoModelMat, vec3(-250.0f, -250.0f, 0.0f));
	logoBuffers = readOBJFile("./models/logo.obj", 0.5f);
	printBuffers(logoBuffers);
	logo = convertToTriangles(logoBuffers); // world coords
	computeNormals(logoBuffers, logo);
	transformedLogo = Transform::transformTriangles(logo, logoModelMat);
	projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);  // screen coords
	if (renderMode == "Rasteriser") {
		rasteriser->drawAllTriangles(window, logoBuffers, projectedLogo, logo, ShadeMode::Flat, true);
	}
	else {
		rayTracer->drawAllTriangles(window, logoBuffers, logo, camera);
	}
}

void loadSphere() {
	sphereModelMat = glm::translate(sphereModelMat, vec3(-60.0f, -150.0f, -280.0f));
	sphereBuffers = readOBJFile("./models/sphere.obj", 30.0f);
	printBuffers(sphereBuffers);
	sphere = convertToTriangles(sphereBuffers); // world coords
	computeNormals(sphereBuffers, sphere);
	transformedSphere = Transform::transformTriangles(sphere, sphereModelMat);
	projectedSphere = Transform::projectTriangles(transformedSphere, viewMat, projMat);  // screen coords
	if (renderMode == "Rasteriser") {
		rasteriser->drawAllTriangles(window, sphereBuffers, projectedSphere, sphere, ShadeMode::Phong, true);
	}
	else {
		rayTracer->drawAllTriangles(window, sphereBuffers, sphere, camera);
	}
}

void update() {
	window.clearPixels();
	if (renderMode == "Rasteriser") {
		rasteriser->resetZBuffer();
	}
	else {
		rayTracer->updateMatrices(viewMat, projMat);
		rayTracer->resetZBuffer();
	}
	viewMat = glm::lookAt(camera, center, cameraUp);
	projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
	projectedcornell = Transform::projectTriangles(transformedCornell, viewMat, projMat);
	projectedSphere = Transform::projectTriangles(transformedSphere, viewMat, projMat);
	if (renderMode == "Rasteriser") {
		rasteriser->updateCamera(camera);
		rasteriser->drawAllTriangles(window, cornellBuffers, projectedcornell, cornell, ShadeMode::Flat, false);
		rasteriser->drawAllTriangles(window, sphereBuffers, projectedSphere, sphere, ShadeMode::Phong, true);
	}
	else {
		rayTracer->drawAllTriangles(window, cornellBuffers, cornell, camera);
		rayTracer->drawAllTriangles(window, sphereBuffers, sphere, camera);
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
			Transform::left(5, camera, cameraUp);
			update();
		}
		else if (event.key.keysym.sym == SDLK_d) {
			Transform::left(-5, camera, cameraUp);
			update();
		}
		else if (event.key.keysym.sym == SDLK_w) {
			Transform::up(-5, camera, cameraUp);
			update();
		}
		else if (event.key.keysym.sym == SDLK_s) {
			Transform::up(5, camera, cameraUp);
			update();
		}
		// Translate Camera
		else if (event.key.keysym.sym == SDLK_UP) {
			Transform::moveUp(-10.0f, camera, center, cameraUp);
			update();
		}
		else if (event.key.keysym.sym == SDLK_DOWN) {
			Transform::moveUp(10.0f, camera, center, cameraUp);
			update();
		}
		else if (event.key.keysym.sym == SDLK_LEFT) {
			Transform::moveRight(10.0f, camera, center, cameraUp);
			update();
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			Transform::moveRight(-10.0f, camera, center, cameraUp);
			update();
		}
	}
	// Dolly Camera
	else if (event.type == SDL_MOUSEWHEEL) {
		if (event.wheel.y > 0) { // scroll up
			Transform::moveForward(30.0f, camera, center, cameraUp);
		}
		else if (event.wheel.y < 0) { // scroll down
			Transform::moveForward(-30.0f, camera, center, cameraUp);
		}
		update();
	}
}


void init() {
	// Load View and Projection Matrices
	rasteriser = new Rasteriser(window.width, window.height);
	rayTracer = new RayTracer(window.width, window.height);
	rayTracer->updateMatrices(viewMat, projMat);
	rasteriser->resetZBuffer();
	rayTracer->resetZBuffer();
	rasteriser->updateCamera(camera);
	rasteriser->updateLights(lights);

	loadCornell();
	loadSphere();
	//loadLogo();
	std::cout << "Initialisation complete." << std::endl;
}

int main(int argc, char* argv[]) {
	init();

	while (true) {
		Uint32 frameStart = SDL_GetTicks();
		if (video) {
			Transform::left(5, camera, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
			window.saveBMP("./Videos/" + std::to_string(frameID++) + ".bmp");
		}
		else {
			if (window.pollForInputEvents(event)) handleEvent(event, window);
		}

		window.renderFrame();
		Uint32 frameTime = SDL_GetTicks() - frameStart;
		if (frameTime < 30) SDL_Delay(30 - frameTime);
	}
	return 0;
}