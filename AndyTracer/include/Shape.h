#pragma once

#include <memory>
#include "Material.h"
#include "defs.h"

class Ray;

class Shape 
{
public:
    virtual ~Shape() = default;

    virtual bool Intersect(const Ray& ray, float tMin, float tMax) const = 0;
    virtual bool Intersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const = 0;

protected:
    std::shared_ptr<Material> material;
};