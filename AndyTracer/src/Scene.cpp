#include "Scene.h"

bool Scene::Intersect(const Ray& ray, float tMin, float tMax) const
{
    for (auto s : others) {
        if (s->Intersect(ray, tMin, tMax))
            return true;
    }

    return false;
}

bool Scene::Intersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const
{
    for (auto s : others) {
        if (s->Intersect(ray, tMin, tMax, info))
            return true;
    }
    return false;
}
