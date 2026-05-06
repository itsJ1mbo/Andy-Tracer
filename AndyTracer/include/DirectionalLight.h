#pragma once

#include "Light.h"

class DirectionalLight : public Light
{
public:
    DirectionalLight(glm::vec3 dir, Color c);

    Color Shade(const Ray& ray, ShapeIntersection& info) override;

private:
    glm::vec3 direction;
};

