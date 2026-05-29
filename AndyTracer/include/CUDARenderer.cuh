#pragma once
#include "defs.h"
#include "Camera.h"
#include "Film.h"

struct Shape;
struct Scene;
struct ShapeIntersection;

class CUDARenderer
{
public:
    CUDARenderer(const Film& f, const Camera& cam, Scene* sc, int r);

    ~CUDARenderer();

    void Render();

private:
    Film film;
    Camera* cameraDevice;
    Shape* sceneShapesDevice;
    Scene* sceneDevice;

    int reflexes; 

    GPUPixel* pixelBufferDevice;

    dim3 blockSize;
    dim3 gridSize;

    const float rayEpsilon = 0.001f;
};