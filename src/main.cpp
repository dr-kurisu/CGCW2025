#include <iostream>
#include <string>
#include "Transform.h"
#include "Utils.h"
#include "Rasterizer.h"
#include "FileLoader.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

#define WIDTH 640
#define HEIGHT 480

SDL_Event event;
DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
// Define View Matrix
glm::vec3 camera(0, 0, 800);
glm::vec3 center(0, 0, 0);
glm::vec3 cameraUp(0, 1, 0);
glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);

// Define Projection Matrix
float fov = 60.0f;
float aspect = (float)window.width / (float)window.height;
float near = 50.0f;
float far = 1500.0f; 
glm::mat4 projMat = glm::perspective(glm::radians(fov), aspect, near, far);


vector<ModelTriangle> logo;
vector<ModelTriangle> transformedLogo;
vector<ModelTriangle> projectedLogo;
mat4 modelMat = mat4(1.0f); // Model Matrix

void handleEvent(SDL_Event event, DrawingWindow& window) {
	if (event.type == SDL_KEYDOWN) {
		// Rotate Camera
		if (event.key.keysym.sym == SDLK_a) {
			Transform::left(5, camera, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
			DrawAllTriangles(window, projectedLogo);
		}
		else if (event.key.keysym.sym == SDLK_d) {
			Transform::left(-5, camera, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
			DrawAllTriangles(window, projectedLogo);
		}
		else if (event.key.keysym.sym == SDLK_w) {
			Transform::up(-5, camera, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
			DrawAllTriangles(window, projectedLogo);
		}
		else if (event.key.keysym.sym == SDLK_s) {
			Transform::up(5, camera, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
		
			DrawAllTriangles(window, projectedLogo);
		}
		// Translate Camera
		else if (event.key.keysym.sym == SDLK_UP) {
			Transform::moveUp(-10.0f, camera, center, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
			DrawAllTriangles(window, projectedLogo);
		}
		else if (event.key.keysym.sym == SDLK_DOWN) {
			Transform::moveUp(10.0f, camera, center, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
			DrawAllTriangles(window, projectedLogo);
		}
		else if (event.key.keysym.sym == SDLK_LEFT) {
			Transform::moveRight(10.0f, camera, center, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
			DrawAllTriangles(window, projectedLogo);
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			Transform::moveRight(-10.0f, camera, center, cameraUp);
			glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
			projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
			DrawAllTriangles(window, projectedLogo);
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
		glm::mat4 viewMat = glm::lookAt(camera, center, cameraUp);
		projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);
		DrawAllTriangles(window, projectedLogo);
	}
}

int main(int argc, char* argv[]) {
	modelMat = glm::translate(modelMat, vec3(-250.0f, -250.0f, 0.0f));
	const Buffers logoBuffers = readOBJFile("./models/logo.obj", 1.0f);
	logo = convertToTriangles(logoBuffers); // world coords
	cout << logo.size() << " triangles loaded." << endl;
	transformedLogo = Transform::transformTriangles(logo, modelMat);
	projectedLogo = Transform::projectTriangles(transformedLogo, viewMat, projMat);  // screen coords

	for (auto t : projectedLogo) {
		drawWireframeTriangle(window, t, Colour(255, 255, 255));
	}
	window.renderFrame();
	while (true) {
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		window.renderFrame();
	}
	return 0;
}
