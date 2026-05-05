#pragma once
#include "Color.h"
class Ray;
struct ShapeIntersection;

class Light
{
public:
	virtual ~Light() = default;

	virtual Color Shade(const Ray& r, ShapeIntersection& info) = 0;

    virtual bool ProjectsShadows() { return false; }

    virtual glm::vec3 ShadowDir(glm::vec3 p) { return glm::vec3(0, 0, 0); }
protected:
    glm::vec3 specularColor;
    glm::vec3 diffuseColor;
    Color lightColor;
};

