#pragma once
#include "Film.h"

class World;
struct ShapeIntersection;

class CUDARenderer
{
public:
    CUDARenderer(const Film& f, int r);

    ~CUDARenderer();

    void Render();

private:
    Film film;
    int reflexes;

    const float rayEpsilon = 0.001f;
};