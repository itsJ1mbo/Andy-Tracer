#include "defs.h"
#include "Film.h"
#include "Camera.h"
#include "Scene.h"
#include "CUDARenderer.cuh"
#include <vector>

#include "Light.h"


int main(void) {

    Film film(800, 600);

    Camera camera(
        Vector3(0, 0, 3),
        Vector3(0, 0, 0),
        Vector3(0, 1, 0),
        film.GetTamX(),
        film.GetTamY(),
        60);

    std::vector<Shape> shapes;

    Shape esfera = CreateSphere(Vector3(2, 0, -2), 1);
    shapes.push_back(esfera);
    Shape esfera2 = CreateSphere(Vector3(0, 0, -4), 1);
    shapes.push_back(esfera2);
    Shape esfera3 = CreateSphere(Vector3(-2, 0, -6), 1);
    shapes.push_back(esfera3);
    Shape suelo = CreateQuad(Vector3(3, -1, 5), Vector3(-1, 0, 0), Vector3(0, 0, 1));
    shapes.push_back(suelo);
    Scene escena;
    escena.shapes = shapes.data();
    escena.shapeCount = (int)shapes.size();

    std::vector<Light> lights;
    Light dirLight = CreateDirectionalLight(Vector3(1, 1, 0), GPUPixel(255));
    lights.push_back(dirLight);
    Light pointLight = CreatePointLight(Vector3(2, 2, 0), GPUPixel(255));
    lights.push_back(pointLight);
    Light pointLight2 = CreatePointLight(Vector3(-2, 2, -3), GPUPixel(255));
    lights.push_back(pointLight2);

    escena.lights = lights.data();
    escena.lightCount = (int)lights.size();

    CUDARenderer* renderer = new CUDARenderer(film, camera, &escena,0);

    bool rendering = true;
    while(rendering)
    {
        renderer->Render();
    }

    delete renderer;

    return 0;
}
