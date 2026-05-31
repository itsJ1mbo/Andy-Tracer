#pragma once
#include "defs.h"

struct Film;
struct Camera;
struct Shape;
struct Scene;
struct Light;
struct ShapeIntersection;
struct BVHNode;

class CUDARenderer
{
public:
    CUDARenderer(Film* f, Camera* cam, Scene* scene);

    ~CUDARenderer();

    void Render();

private:
    Film* film;
    Camera* cameraHost;

    //punteros a memoria de GPU
    //camara
    Camera* cameraDevice;
    //shapes de la escena
    Shape* sceneShapesDevice;
    // Nodos de la escena
	BVHNode* sceneBVHDevice;
    //luces de la escena
    Light* sceneLightsDevice;
    //la escena en si
    Scene* sceneDevice;
    //buffer de pixeles de GPU
    GPUPixel* pixelBufferDevice;

    //tamano con el que se inicializa el kernel
    dim3 blockSize;
    dim3 gridSize;
};