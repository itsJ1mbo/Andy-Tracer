#pragma once
#include "Shape.h"
class Scene : public Shape
{
public:
    ~Scene() {
        for (auto s : others) {
            delete s;
            s = nullptr;
        }
    }

    inline void Add(Shape* s) { others.push_back(s); }

    bool Intersect(const Ray& ray, float tMin, float tMax) const override;
    bool Intersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const override;

private:
    std::list<Shape*> others;
};

