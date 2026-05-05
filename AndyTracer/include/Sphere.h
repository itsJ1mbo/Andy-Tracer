#pragma once
#include "Shape.h"
#include "glm/vec3.hpp"
#include "glm/geometric.hpp"

class Sphere : public Shape 
{
public:
    Sphere(glm::vec3 c, float r, std::shared_ptr<Material> m);

    bool Intersect(const Ray& ray, float tMin, float tMax) const override;
    bool Intersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const override;
private:
    glm::vec3 center;
    float radius;
};