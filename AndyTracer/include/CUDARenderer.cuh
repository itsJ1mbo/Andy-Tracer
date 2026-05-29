#pragma once
#include "defs.h"

struct Film;
struct Camera;
struct Shape;
struct Scene;
struct Light;
struct ShapeIntersection;

class CUDARenderer
{
public:
    CUDARenderer(Film* f, Camera* cam, Scene* sc, int r);

    ~CUDARenderer();

    void Render();

private:
    Film* film;
    Camera* cameraDevice;
    Shape* sceneShapesDevice;
    Scene* sceneDevice;
    Light* sceneLightsDevice;

    int reflexes; 

    GPUPixel* pixelBufferDevice;

    dim3 blockSize;
    dim3 gridSize;
};