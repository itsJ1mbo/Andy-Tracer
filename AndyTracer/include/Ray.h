#pragma once

#include "defs.h"

struct Ray
{
    Vector3 origin;
    Vector3 direction;

    __device__ Ray(const Vector3& pos, const Vector3& dir)
    {
        origin = pos;
        direction = dir;
    }

    __device__ Vector3 At(float t) const
    {
        return origin + t * direction;
    }
};