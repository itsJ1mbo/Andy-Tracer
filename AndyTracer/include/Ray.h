#pragma once
#include "defs.h"

struct Ray
{
    Vector3 position;
    Vector3 direction;

    __device__ Ray(Vector3 pos, Vector3 dir)
    {
        position = pos;
        direction = dir;
    }

    __device__ Vector3 At(float t)
    {
        return position + t * direction;
    }
};