#include <iostream>
#include <string>
#include "Transform.h"
#include "Utils.h"
#include "Rasteriser.h"
#include "RayTracer.h"
#include "Renderer.h"
#include "FileLoader.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

#define WIDTH 640
#define HEIGHT 480

// Key Parameters
bool video = false;
const string renderMode = "RayTracer";
// const string renderMode = "Rasteriser";

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

mat4 logoModelMat = mat4(1.0f); // Model Matrix for logo
mat4 cornellModelMat = mat4(1.0f); // Model Matrix for cornell

// Define Rasteriser and RayTracer
Renderer* renderer;

int frameID = 0;

void loadCornell() {
	cornellModelMat = glm::translate(cornellModelMat, vec3(0.0f, 0.0f, -300.0f));
	cornellBuffers = readOBJFile("./models/cornell-box.obj", 50.0f);
	printBuffers(cornellBuffers);
	cornell = convertToTriangles(cornellBuffers); // world coords
	transformedCornell = Transform::transformTriangles(cornell, cornellModelMat);
	projectedcornell = Transform::projectTriangles(transformedCornell, viewMat, projMat);  // screen coords
	if (renderMode == "Rasteriser") {
		renderer->drawAllTriangles(window, cornellBuffers, projectedcornell);
	}
	else {
		renderer->drawAllTriangles(window, cornellBuffers, cornell);
	}
}

void loadLogo() {
	logoModelMat = glm::translate(logoModelMat, vec3(-250.0f, -250.0f, 0.0f));
	logoBuffers = readOBJFile("./models/logo.obj", 0.5f);
	printBuffers(logoBuffers);
	logo = convertToTriangles(logoBuffers); // world coords
	transformedLogo = Transform::transformTriangles(logo, logoModelMat);
	projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);  // screen coords
	if (renderMode == "Rasteriser") {
		renderer->drawAllTriangles(window, logoBuffers, projectedLogo);
	}
	else {
		renderer->drawAllTriangles(window, logoBuffers, logo);
	}
}

void update() {
	window.clearPixels();
	renderer->resetZBuffer();
	renderer->updateMatrices(viewMat, projMat);
	glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
	projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
	projectedcornell = Transform::projectTriangles(transformedCornell, viewMat, projMat);
	if (renderMode == "Rasteriser") {
		renderer->drawAllTriangles(window, cornellBuffers, projectedcornell);
		//renderer->drawAllTriangles(window, logoBuffers, projectedLogo);
		return;
	}
	else {
		renderer->drawAllTriangles(window, cornellBuffers, cornell);
		//renderer->drawAllTriangles(window, logoBuffers, logo);
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
	if (renderMode == "RayTracer") {
		renderer = new RayTracer(WIDTH, HEIGHT);
	}
	else if (renderMode == "Rasteriser") {
		renderer = new Rasteriser(WIDTH, HEIGHT);
	}
	else {
		cerr << "Invalid render mode" << endl;
		exit(1);
	}
	// Load View and Projection Matrices
	renderer->updateMatrices(viewMat, projMat);
	renderer->resetZBuffer();
	loadCornell();
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