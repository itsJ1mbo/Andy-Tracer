#pragma once
#include "Shape.h"
#include "glm/vec3.hpp"
#include "glm/geometric.hpp"

class Square : public Shape
{
public:
    Square(glm::vec3 p, glm::vec3 h, glm::vec3 w, const std::shared_ptr<Material>& m);

    bool Intersect(const Ray& ray, float tMin, float tMax) const override;
    bool Intersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const override;
private:
    glm::vec3 corner;
    glm::vec3 width;
    glm::vec3 height;
    glm::vec3 normal;
};