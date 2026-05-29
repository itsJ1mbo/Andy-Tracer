#pragma once
#include "Shape.h"

struct Light;

struct Scene : Shape
{
    int shapeCount;
    Shape* shapes;

    int lightCount;
    Light* lights;

    __host__ Scene() : shapeCount(0), shapes(nullptr), lightCount(0), lights(nullptr) {}

    __device__ bool Intersect(Ray& ray, float tMin, float tMax, ShapeIntersection& info)
    {
        for(int i = 0; i < shapeCount; i++)
        {
            if (shapes[i].Intersect(ray, tMin, tMax, info))
                return true;
        }
        return false;
    }
};