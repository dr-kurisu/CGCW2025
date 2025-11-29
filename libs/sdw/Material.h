#ifndef MATERIAL_H
#define MATERIAL_H

#include "TextureMap.h"
#include "Colour.h"
#include <string>

struct Material {
    std::string name;
    Colour Kd;            // diffuse
    std::string mapName;   // diffuse texture
	std::string type;      // material type
    bool isReflective;

    Material(std::string name) : name(name), Kd(255, 255, 255), mapName(""), isReflective(false), type("kd_map") {}
	Material() : name(""), Kd(255, 255, 255), mapName(""), isReflective(false), type("kd") {}
};

#endif