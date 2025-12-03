#pragma once

#include <iostream>
#include <algorithm>

struct Colour {
	std::string name;
	int red{};
	int green{};
	int blue{};
	Colour();
	Colour(int r, int g, int b);
	Colour(std::string n, int r, int g, int b);

	// Add multiplication
	Colour operator*(float factor) const {
		int r = std::clamp(static_cast<int>(red * factor), 0, 255);
		int g = std::clamp(static_cast<int>(green * factor), 0, 255);
		int b = std::clamp(static_cast<int>(blue * factor), 0, 255);
		return Colour(r, g, b);
	}
	// Add addition
	Colour operator+(const Colour &other) const {
		int r = std::clamp(red + other.red, 0, 255);
		int g = std::clamp(green + other.green, 0, 255);
		int b = std::clamp(blue + other.blue, 0, 255);
		return Colour(r, g, b);
	}
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);
