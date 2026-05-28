#include "CUDARenderer.cuh"
#include <cuda.h>
#include <cuda_runtime.h>
#define GLM_FORCE_CUDA
#include <glm/glm.hpp>
#include "World.h"
#include "Scene.h"
#include "Light.h"
#include "defs.h"
#include <cstdio>

__device__ void RayColor(GPUPixel* buffer, int width, int height)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    //printf("Pixel: (%d, %d)\n", x, y);

    if (x < width && y < height)
    {
        //indice del pixel
        int workId = y * width + x;

        buffer[workId].r = (int)(255.99f * ((float)x / (width - 1)));
        buffer[workId].g = (int)(255.99f * ((float)y / (height - 1)));
        buffer[workId].b = 0;
        buffer[workId].a = 255;
    }
}

__global__ void SamplePixel(GPUPixel* buffer, int width, int height)
{
    RayColor(buffer, width, height);
}

CUDARenderer::CUDARenderer(const Film& f, const Camera& c, World* w, int r, int s) : film(f), camera(c), world(w), reflexes(r), samples(s)
{

}

CUDARenderer::~CUDARenderer()
{
    delete world;
}

void CUDARenderer::Render()
{
    dim3 blockSize(16, 16);
    dim3 gridSize((film.GetTamX() + blockSize.x - 1) / blockSize.x,
        (film.GetTamY() + blockSize.y - 1) / blockSize.y);

    //printf("Grid: (%d, %d, %d), Block: (%d, %d, %d)\n", gridSize.x, gridSize.y, gridSize.z, blockSize.x, blockSize.y, blockSize.z);

    GPUPixel* buffer = nullptr;
    cudaMallocManaged(&buffer, film.GetTamX() * film.GetTamY() * sizeof(GPUPixel));

    SamplePixel <<<gridSize, blockSize >>>(buffer, film.GetTamX(), film.GetTamY());

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA Launch Error: %s\n", cudaGetErrorString(err));
        return;
    }

    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("CUDA Sync Error: %s\n", cudaGetErrorString(err));
    }

    //printf("Frame terminado\n");

    film.CopyBuffer(buffer);

    film.Display();

    cudaFree(buffer);
}