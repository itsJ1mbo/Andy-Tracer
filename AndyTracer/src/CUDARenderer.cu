#include "CUDARenderer.cuh"
#include <cuda_runtime.h>
#include "Camera.h"
#include "config.h"
#include "Scene.h"
#include "Film.h"
#include "AABB.h"

__device__ Vector3 RayColor(const Ray& ray, Scene* scene);

// No se puede usar la cabecera <random> en CUDA así que implementamos un generador pseudo-aleatorio muy simple
// Asi evitamos enlazar cuRAND que para un aleatorio sencillo no merece la pena
__device__ float GetRandomFloat(unsigned int& seed) {
    seed = 1664525 * seed + 1013904223;
    return (static_cast<float>(seed & 0x00FFFFFF) / static_cast<float>(0x01000000));
}

__device__ Vector3 Reflect(const Ray& ray, const ShapeIntersection& info)
{
    Vector3 I = normalize(ray.direction);
    return normalize(I - (2.0f * dot(I, info.normal)) * info.normal);
}

__device__ Vector3 Shade(const Ray& ray, Scene* scene, ShapeIntersection& info)
{
    Vector3 ret(0);

    for(int i = 0; i < scene->lightCount; i++)
    {
        if(scene->lights[i].projectsShadow)
        {
            Vector3 dir = scene->lights[i].GetShadowDir(info.position);
            //esta feo pero no queria tener que pasar esto a traves de todo el kernel
            Ray shadowRay(info.position + dir * config::RAY_EPSILON, dir);
            ShapeIntersection shadowInfo;
            if (scene->IntersectBVH(shadowRay, 0.0f, INFINITY, shadowInfo))
            {
                continue; 
            }
        }
        ret = ret + scene->lights[i].Shade(ray, info);
    }

    ret = Vector3(fminf(1.0f, ret.x), fminf(1.0f, ret.y), fminf(1.0f, ret.z));
    return ret;
}

__device__ Vector3 RayColor(const Ray& ray, Scene* scene)
{
    Vector3 finalColor(0, 0, 0);
    Vector3 rayEnergy(1.0f, 1.0f, 1.0f);
    Ray currentRay = ray;

    for (int r = 0; r <= config::MAX_REFLEXES; r++)
    {
        ShapeIntersection closestInfo;

        if (!scene->IntersectBVH(currentRay, 0.0f, INFINITY, closestInfo))
        {
            break;
        }

		// Color con luz y reflexion iterativo para que la GPU no llore
        Vector3 light = Shade(currentRay, scene, closestInfo);
        finalColor = finalColor + rayEnergy * light;

        float reflexFactor = closestInfo.material.GetReflexFactor();
        if (reflexFactor <= 0.0f)
        {
            break;
        }

        rayEnergy *= reflexFactor;
        Vector3 reflectDir = Reflect(currentRay, closestInfo);
        currentRay = Ray(closestInfo.position + reflectDir * config::RAY_EPSILON, reflectDir);
    }

    return finalColor;
}

__global__ void SamplePixel(GPUPixel* buffer, Camera* cam, Scene* scene)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < cam->width && y < cam->height)
    {
        Vector3 finalColor = Vector3(0, 0, 0);
        //indice del pixel
        int workId = y * cam->width + x;
        // Multi-sampling
        int samples = config::SAMPLES;

		unsigned int seed = workId; // Semilla diferente para cada pixel

        for (int s = 0; s < samples; s++) {
            const Ray ray_primary = cam->GetRay(x + GetRandomFloat(seed), y + GetRandomFloat(seed), seed);
            finalColor += RayColor(ray_primary, scene);
        }
        finalColor /= config::SAMPLES;

        // Se calcula en floats y vectores y solo se convierte a enteros (GPUPixel) al final para no perder precision
        finalColor.x = fminf(1.0f, finalColor.x);
        finalColor.y = fminf(1.0f, finalColor.y);
        finalColor.z = fminf(1.0f, finalColor.z);

        buffer[workId] = GPUPixel(
            finalColor.x * 255.0f + 0.5f,
            finalColor.y * 255.0f + 0.5f,
            finalColor.z * 255.0f + 0.5f,
            255
        );
    }
}

CUDARenderer::CUDARenderer(Film* f, Camera* cam, Scene* scene) : film(f)
{
    cameraHost = cam;
    //tamano con el que lanzaremos el kernel
    //16*16 = 256 hilos por bloque
    blockSize = { 16, 16 };
    gridSize = { (film->GetTamX() + blockSize.x - 1) / blockSize.x,
        (film->GetTamY() + blockSize.y - 1) / blockSize.y };

    //reservamos espacio en gpu para el buffer de pixeles
    pixelBufferDevice = nullptr;
    cudaMallocManaged(&pixelBufferDevice, film->GetTamX() * film->GetTamY() * sizeof(GPUPixel));

    //puntero en memoria de gpu a la camara
    cameraDevice = nullptr;
    cudaMalloc(&cameraDevice, sizeof(Camera));
    cudaMemcpy(cameraDevice, cam, sizeof(Camera), cudaMemcpyHostToDevice);

    //puntero en gpu a la escena
    //puntero al array de shapes de la escena que pasamos
    size_t shapesSize = sizeof(Shape) * scene->shapeCount;
    sceneShapesDevice = nullptr;
    cudaMalloc(&sceneShapesDevice, shapesSize);
    cudaMemcpy(sceneShapesDevice, scene->shapes, shapesSize, cudaMemcpyHostToDevice);
	// Puntero al array de nodos del BVH de la escena que pasamos
	size_t nodesSize = sizeof(BVHNode) * scene->nodesCount;
    sceneBVHDevice = nullptr;
    cudaMalloc(&sceneBVHDevice, nodesSize);
    cudaMemcpy(sceneBVHDevice, scene->bvhNodes, nodesSize, cudaMemcpyHostToDevice);
    //puntero al array de luces de la escena
    size_t lightsSize = sizeof(Light) * scene->lightCount;
    sceneLightsDevice = nullptr;
    cudaMalloc(&sceneLightsDevice, lightsSize);
    cudaMemcpy(sceneLightsDevice, scene->lights, lightsSize, cudaMemcpyHostToDevice);
    //creamos una copia temporal de la escena pero con el array de shapes y de luces de la gpu (que acabamos de copiar)
    //(necesario para el siguiente paso)
    Scene tempScene;
    tempScene.shapes = sceneShapesDevice;
    tempScene.shapeCount = scene->shapeCount;
    tempScene.lights = sceneLightsDevice;
    tempScene.lightCount = scene->lightCount;
    tempScene.bvhNodes = sceneBVHDevice;
    tempScene.nodesCount = scene->nodesCount;
    //copiamos la escena temporal a la escena de gpu (si copiasemos la escena que pasamos entonces copiariamos los arrays
    //que contiene la escena de cpu al puntero de la escena de gpu, que es algo que crashearia el kernel)
    sceneDevice = nullptr;
    cudaMalloc(&sceneDevice, sizeof(Scene));
    cudaMemcpy(sceneDevice, &tempScene, sizeof(Scene), cudaMemcpyHostToDevice);
}

CUDARenderer::~CUDARenderer()
{
    //liberamos el espacio en gpu usado
    cudaFree(pixelBufferDevice);
    cudaFree(cameraDevice);
    cudaFree(sceneBVHDevice);
    cudaFree(sceneShapesDevice);
    cudaFree(sceneLightsDevice);
    cudaFree(sceneDevice);
}

void CUDARenderer::Render()
{
    //actualizamos la posicion de la camara de la cpu a la gpu
    cudaMemcpy(cameraDevice, cameraHost, sizeof(Camera), cudaMemcpyHostToDevice);
    //lanzamos el kernel
    SamplePixel <<<gridSize, blockSize >>>(pixelBufferDevice, cameraDevice, sceneDevice);

    //por si sale algun error
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA Launch Error: %s\n", cudaGetErrorString(err));
        return;
    }

    //esperamos a que acaben todos los hilos
    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("CUDA Sync Error: %s\n", cudaGetErrorString(err));
    }

    //copiamos buffer de pixeles al de la ventana (no podemos usar el de la ventana directamente porque es de un tipo propio de la libreria)
    film->CopyBuffer(pixelBufferDevice);

    //mostramos el frame
    film->Display();
}