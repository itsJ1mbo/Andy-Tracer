#include "CUDARenderer.cuh"
#include <cuda_runtime.h>
#include "Camera.h"
#include "Scene.h"
#include "Film.h"
#include <cstdio>

__device__ GPUPixel Shade(Ray ray, Scene* scene, ShapeIntersection& info)
{
    Vector3 ret(0);

    for(int i = 0; i < scene->lightCount; i++)
    {
        if(scene->lights[i].projectsShadow)
        {
            Vector3 dir = scene->lights[i].GetShadowDir(info.position);
            //no queria tener que pasar esto a traves de todo el kernel
            float rayEpsilon = 0.01f;
            Ray shadowRay(info.position + dir * rayEpsilon, dir);
            ShapeIntersection shadowInfo;
            if(scene->Intersect(shadowRay, 0, INFINITY, shadowInfo))
            {
                continue;
            }
        }
        ret = ret + scene->lights[i].Shade(ray, info);
    }

    ret = Vector3(fminf(1.0f, ret.x), fminf(1.0f, ret.y), fminf(1.0f, ret.z));
    return GPUPixel(ret.x * 255.0f + 0.5f, ret.y * 255.0f + 0.5f, ret.z * 255.0f + 0.5f, 255);
}

__device__ GPUPixel RayColor(Ray ray, Scene* scene)
{
    for(int i = 0; i < scene->shapeCount; i++)
    {
        ShapeIntersection info;
        if(scene->shapes[i].Intersect(ray, 0, INFINITY, info))
        {
            return Shade(ray, scene, info);
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

CUDARenderer::CUDARenderer(Film* f, Camera* cam, Scene* sc, int r) : film(f),reflexes(r)
{
    cameraHost = cam;
    //tamano con el que lanzaremos el kernel
    //16*16 = 256 hilos por bloque
    blockSize = { 16, 16 };
    gridSize = { (film->GetTamX() + blockSize.x - 1) / blockSize.x,
        (film->GetTamY() + blockSize.y - 1) / blockSize.y };

    //reservamos espacio para el buffer de pixeles
    pixelBufferDevice = nullptr;
    cudaMallocManaged(&pixelBufferDevice, film->GetTamX() * film->GetTamY() * sizeof(GPUPixel));

    //puntero en memoria de gpu a la camara
    cameraDevice = nullptr;
    cudaMalloc(&cameraDevice, sizeof(Camera));
    cudaMemcpy(cameraDevice, cam, sizeof(Camera), cudaMemcpyHostToDevice);

    //puntero en gpu a la escena
    //puntero al array de shapes de la escena que pasamos
    size_t shapesSize = sizeof(Shape) * sc->shapeCount;
    sceneShapesDevice = nullptr;
    cudaMalloc(&sceneShapesDevice, shapesSize);
    cudaMemcpy(sceneShapesDevice, sc->shapes, shapesSize, cudaMemcpyHostToDevice);
    //puntero al array de luces de la escena
    size_t lightsSize = sizeof(Light) * sc->lightCount;
    sceneLightsDevice = nullptr;
    cudaMalloc(&sceneLightsDevice, lightsSize);
    cudaMemcpy(sceneLightsDevice, sc->lights, lightsSize, cudaMemcpyHostToDevice);
    //creamos una copia temporal de la escena pero con el array de shapes y de luces de la gpu (que acabamos de copiar)
    Scene tempScene;
    tempScene.shapes = sceneShapesDevice;
    tempScene.shapeCount = sc->shapeCount;
    tempScene.lights = sceneLightsDevice;
    tempScene.lightCount = sc->lightCount;
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
    cudaFree(sceneLightsDevice);
    cudaFree(sceneDevice);
}

void CUDARenderer::Render()
{
    cudaMemcpy(cameraDevice, cameraHost, sizeof(Camera), cudaMemcpyHostToDevice);
    //lanzamos el kernel
    SamplePixel <<<gridSize, blockSize >>>(pixelBufferDevice, film->GetTamX(), film->GetTamY(), cameraDevice, sceneDevice);

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
    film->CopyBuffer(pixelBufferDevice);

    //mostramos el frame
    film->Display();
}