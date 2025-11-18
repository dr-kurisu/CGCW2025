#ifndef MATERIAL_H
#define MATERIAL_H

#include "TextureMap.h"
#include "Colour.h"
#include <string>

struct Material {
    std::string name;
    Colour Kd;            // diffuse
    std::string mapName;   // diffuse texture
    bool isReflective;

    Material(std::string name) : name(""), Kd(), mapName(name), isReflective(false) {}
	Material() : name(""), Kd(), mapName(""), isReflective(false) {}
};

#endif