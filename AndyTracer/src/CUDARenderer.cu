#include "CUDARenderer.cuh"
#include <cuda.h>
#include <cuda_runtime.h>
#include "Ray.h"
#include "Scene.h"
#include <cstdio>


__device__ GPUPixel RayColor(Ray ray, Scene* scene)
{
    for(int i = 0; i < scene->count; i++)
    {
        ShapeIntersection info;
        if(scene->shapes[i].Intersect(ray, 0, INFINITY, info))
        {
            return GPUPixel(255);
        }
    }
    return GPUPixel(0);
}

__global__ void SamplePixel(GPUPixel* buffer, int width, int height, Camera* cam, Scene* scene)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    //printf("Pixel: (%d, %d)\n", x, y);

    if (x < width && y < height)
    {
        //indice del pixel
        int workId = y * width + x;

        buffer[workId] = RayColor(cam->GetRay(x, y), scene);
    }
}

CUDARenderer::CUDARenderer(const Film& f, const Camera& cam, Scene* sc, int r) : film(f),reflexes(r)
{
    //tamano con el que lanzaremos el kernel
    //16*16 = 256 hilos por bloque
    blockSize = { 16, 16 };
    gridSize = { (film.GetTamX() + blockSize.x - 1) / blockSize.x,
        (film.GetTamY() + blockSize.y - 1) / blockSize.y };

    //reservamos espacio para el buffer de pixeles
    pixelBufferDevice = nullptr;
    cudaMallocManaged(&pixelBufferDevice, film.GetTamX() * film.GetTamY() * sizeof(GPUPixel));

    //puntero en memoria de gpu a la camara
    cameraDevice = nullptr;
    cudaMallocManaged(&cameraDevice, sizeof(Camera));
    cameraDevice->position = cam.position;
    cameraDevice->delta_x = cam.delta_x;
    cameraDevice->delta_y = cam.delta_y;
    cameraDevice->position_top_left = cam.position_top_left;

    //puntero en gpu a la escena
    //puntero al array de shapes de la escena que pasamos
    size_t shapeSize = sizeof(Shape) * sc->count;
    sceneShapesDevice = nullptr;
    cudaMalloc(&sceneShapesDevice, shapeSize);
    cudaMemcpy(sceneShapesDevice, sc->shapes, shapeSize, cudaMemcpyHostToDevice);
    //creamos una copia temporal de la escena pero con el array de shapes de la gpu (el que acabamos de copiar)
    Scene tempScene;
    tempScene.shapes = sceneShapesDevice;
    tempScene.count = sc->count;
    //copiamos la escena temporal a la escena de gpu
    sceneDevice = nullptr;
    cudaMalloc(&sceneDevice, sizeof(Scene));
    cudaMemcpy(sceneDevice, &tempScene, sizeof(Scene), cudaMemcpyHostToDevice);
}

CUDARenderer::~CUDARenderer()
{
    cudaFree(pixelBufferDevice);
    cudaFree(cameraDevice);
    cudaFree(sceneShapesDevice);
    cudaFree(sceneDevice);
}

void CUDARenderer::Render()
{
    //lanzamos el kernel
    SamplePixel <<<gridSize, blockSize >>>(pixelBufferDevice, film.GetTamX(), film.GetTamY(), cameraDevice, sceneDevice);

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA Launch Error: %s\n", cudaGetErrorString(err));
        return;
    }

    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("CUDA Sync Error: %s\n", cudaGetErrorString(err));
    }

    //copiamos buffer de pixeles al de la ventana (no podemos usar el de la ventana directamente porque es de un tipo propio de la libreria)
    film.CopyBuffer(pixelBufferDevice);

    //mostramos el frame
    film.Display();
}