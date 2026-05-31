#pragma once

#include "Ray.h"

struct AABB
{
    Vector3 bMin;
    Vector3 bMax;

    __device__ bool Intersect(const Ray& ray, float tMin, float tMax) const
    {
        for (int a = 0; a < 3; a++) 
        {
            float invD = 1.0f / (a == 0 ? ray.direction.x : (a == 1 ? ray.direction.y : ray.direction.z));
            float t0 = ((a == 0 ? bMin.x : (a == 1 ? bMin.y : bMin.z)) - (a == 0 ? ray.origin.x : (a == 1 ? ray.origin.y : ray.origin.z))) * invD;
            float t1 = ((a == 0 ? bMax.x : (a == 1 ? bMax.y : bMax.z)) - (a == 0 ? ray.origin.x : (a == 1 ? ray.origin.y : ray.origin.z))) * invD;

            if (invD < 0.0f) { float temp = t0; t0 = t1; t1 = temp; }
            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax <= tMin) return false;
        }
        return true;
    }

    static AABB SurroundingBox(const AABB& a, const AABB& b)
    {
        return AABB{
            Vector3(fminf(a.bMin.x, b.bMin.x), fminf(a.bMin.y, b.bMin.y), fminf(a.bMin.z, b.bMin.z)),
            Vector3(fmaxf(a.bMax.x, b.bMax.x), fmaxf(a.bMax.y, b.bMax.y), fmaxf(a.bMax.z, b.bMax.z))
        };
    }
};

struct BVHNode
{
    AABB bounds;
    int leftChild;
    int rightChild;
    int firstShape;
    bool isLeaf;
};