#pragma once
#include "Light.h"
class PointLight : public Light
{
public:
    PointLight(glm::vec3 pos, Color c);

    Color Shade(const Ray& ray, ShapeIntersection& info) override;

    bool ProjectsShadows() override;

    glm::vec3 ShadowDir(glm::vec3 p) override;

private:
    glm::vec3 position;
};

