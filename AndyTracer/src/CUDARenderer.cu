#include "CUDARenderer.cuh"
#include <cuda.h>
#include <cuda_runtime.h>
#define GLM_FORCE_CUDA
#include <glm/glm.hpp>
#include "World.h"
#include "Scene.h"
#include "Light.h"
#include <cstdio>

__device__ void RayColor()
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    printf("Pixel: (%d, %d)\n", x, y);
}

__global__ void SamplePixel()
{
    RayColor();
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
    // Fix: Use curly braces or constructor parentheses
    dim3 blockSize(16, 16);
    dim3 gridSize((800 + blockSize.x - 1) / blockSize.x,
        (600 + blockSize.y - 1) / blockSize.y);

    printf("Grid: (%d, %d, %d), Block: (%d, %d, %d)\n", gridSize.x, gridSize.y, gridSize.z, blockSize.x, blockSize.y, blockSize.z);

    SamplePixel <<<gridSize, blockSize >>>();

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA Launch Error: %s\n", cudaGetErrorString(err));
        return;
    }

    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("CUDA Sync Error: %s\n", cudaGetErrorString(err));
    }

    printf("Frame terminado\n");
}