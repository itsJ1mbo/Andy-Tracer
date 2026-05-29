#include "CUDARenderer.cuh"
#include <cuda.h>
#include <cuda_runtime.h>
#include <cstdio>

__device__ GPUPixel RayColor(Ray ray)
{
    return GPUPixel(150);
}

__global__ void SamplePixel(GPUPixel* buffer, int width, int height, Camera* cam)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    //printf("Pixel: (%d, %d)\n", x, y);

    if (x < width && y < height)
    {
        //indice del pixel
        int workId = y * width + x;

        buffer[workId] = RayColor(cam->GetRay(x, y));
    }
}

CUDARenderer::CUDARenderer(const Film& f, const Camera& cam, int r) : film(f),reflexes(r)
{
    //tamano con el que lanzaremos el kernel
    //16*16 = 256 hilos por bloque
    blockSize = { 16, 16 };
    gridSize = { (film.GetTamX() + blockSize.x - 1) / blockSize.x,
        (film.GetTamY() + blockSize.y - 1) / blockSize.y };

    //reservamos espacio para el buffer de pixeles
    pixelBuffer = nullptr;
    cudaMallocManaged(&pixelBuffer, film.GetTamX() * film.GetTamY() * sizeof(GPUPixel));

    camera = nullptr;
    cudaMallocManaged(&camera, sizeof(Camera));
    camera->position = cam.position;
    camera->delta_x = cam.delta_x;
    camera->delta_y = cam.delta_y;
    camera->position_top_left = cam.position_top_left;
}

CUDARenderer::~CUDARenderer()
{
    cudaFree(pixelBuffer);
}

void CUDARenderer::Render()
{
    //lanzamos el kernel
    SamplePixel <<<gridSize, blockSize >>>(pixelBuffer, film.GetTamX(), film.GetTamY(), camera);

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
    film.CopyBuffer(pixelBuffer);

    //mostramos el frame
    film.Display();
}