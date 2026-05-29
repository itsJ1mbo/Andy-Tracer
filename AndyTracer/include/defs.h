#pragma once
#include "cuda_runtime.h"

using integer8 = unsigned char;

struct GPUPixel
{
    integer8 b, g, r, a;

    __host__ __device__ GPUPixel(integer8 n)
    {
        r = n;
        g = n;
        b = n;
        a = n;
    }

    __host__ __device__ GPUPixel(integer8 red, integer8 green, integer8 blue, integer8 alpha)
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }
};
