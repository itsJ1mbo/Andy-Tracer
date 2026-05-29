#pragma once
#include "defs.h"
#include "Camera.h"
#include "Film.h"

class World;
struct ShapeIntersection;

class CUDARenderer
{
public:
    CUDARenderer(const Film& f, const Camera& cam, int r);

    ~CUDARenderer();

    void Render();

private:
    Film film;
    Camera* camera;
    int reflexes; 

    GPUPixel* pixelBuffer;

    dim3 blockSize;
    dim3 gridSize;

    const float rayEpsilon = 0.001f;
};