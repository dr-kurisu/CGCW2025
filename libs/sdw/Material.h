#ifndef MATERIAL_H
#define MATERIAL_H

#include "TextureMap.h"
#include "Colour.h"
#include <string>

struct Material {
    std::string name;
    Colour Kd;            // diffuse
    std::string mapName;   // diffuse texture
	std::string type;      // material type "kd, kd_map, reflect", "metallic"
	Colour metalColour; // colour for metallic
	float metallic;      // metallic factor
	float reflectStrength; // reflection strength
	float refractFactor; // refraction factor
	float transparency = 0.0f;  // transparency factor

    Material(std::string name) : name(name), Kd(255, 255, 255), mapName(""), type("kd"), reflectStrength(0.0f), metalColour(), metallic(), refractFactor(0.0f){}
	Material() : name(""), Kd(255, 255, 255), mapName(""), type("kd"), reflectStrength(0.0f), metalColour(), metallic(), refractFactor(0.0f) {}
};

#endif