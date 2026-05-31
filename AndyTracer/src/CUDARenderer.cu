#include "CUDARenderer.cuh"
#include <cuda_runtime.h>
#include "Camera.h"
#include "Scene.h"
#include "Film.h"

__device__ Vector3 RayColor(Ray ray, Scene* scene, int& reflexes);

// No se puede usar la cabecera <random> en CUDA así que implementamos un generador pseudo-aleatorio muy simple
// Asi evitamos enlazar cuRAND que para un aleatorio sencillo no merece la pena
__device__ float GetRandomFloat(unsigned int& seed) {
    seed = 1664525 * seed + 1013904223;
    return ((float)(seed & 0x00FFFFFF) / (float)0x01000000);
}

__device__ Vector3 Reflect(Ray ray, ShapeIntersection& info)
{
    Vector3 I = normalize(ray.direction);
    return normalize(I - (2.0f * dot(I, info.normal)) * info.normal);
}

__device__ Vector3 Shade(Ray ray, Scene* scene, ShapeIntersection& info, int& reflexes)
{
    Vector3 ret(0);
    float rayEpsilon = 0.01f;

    for(int i = 0; i < scene->lightCount; i++)
    {
        if(scene->lights[i].projectsShadow)
        {
            Vector3 dir = scene->lights[i].GetShadowDir(info.position);
            //esta feo pero no queria tener que pasar esto a traves de todo el kernel
            Ray shadowRay(info.position + dir * rayEpsilon, dir);
            ShapeIntersection shadowInfo;
            if(scene->Intersect(shadowRay, 0, INFINITY, shadowInfo))
            {
                continue;
            }
        }
        ret = ret + scene->lights[i].Shade(ray, info);
    }

    if (info.material.GetReflexFactor() > 0 && reflexes > 0) {
        reflexes--;
        auto dir = Reflect(ray, info);
        Ray reflejo = Ray(info.position + dir * rayEpsilon, dir);
        ret += info.material.GetReflexFactor() * RayColor(reflejo, scene, reflexes);
    }

    ret = Vector3(fminf(1.0f, ret.x), fminf(1.0f, ret.y), fminf(1.0f, ret.z));
    return ret;
}

__device__ Vector3 RayColor(Ray ray, Scene* scene, int& reflexes)
{
    ShapeIntersection closestInfo;
    bool hitAnything = false;
    float closestT = INFINITY;

    for(int i = 0; i < scene->shapeCount; i++)
    {
        ShapeIntersection info;
        if(scene->shapes[i].Intersect(ray, 0, closestT, info))
        {
            closestT = info.t;
            closestInfo = info;
            hitAnything = true;
        }
    }

    if (hitAnything) {
        return Shade(ray, scene, closestInfo, reflexes);
    }

    return Vector3(0, 0, 0);
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
        int samples = 10;

		unsigned int seed = workId; // Semilla diferente para cada pixel

        for (int s = 0; s < samples; s++) {
            const Ray ray_primary = cam->GetRay(x + GetRandomFloat(seed), y + GetRandomFloat(seed));
            int reflexes = 10;
            finalColor += RayColor(ray_primary, scene, reflexes);
        }
        finalColor /= samples;

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

CUDARenderer::CUDARenderer(Film* f, Camera* cam, Scene* sc, int r) : film(f),reflexes(r)
{
	// Para que aguante la recursividad subimos el limite de pila
    cudaError_t limitErr = cudaDeviceSetLimit(cudaLimitStackSize, 8192);
    if (limitErr != cudaSuccess) {
        printf("Error subiendo el limite de pila: %s\n", cudaGetErrorString(limitErr));
    }

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
    //(necesario para el siguiente paso)
    Scene tempScene;
    tempScene.shapes = sceneShapesDevice;
    tempScene.shapeCount = sc->shapeCount;
    tempScene.lights = sceneLightsDevice;
    tempScene.lightCount = sc->lightCount;
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