#include "defs.h"
#include "Film.h"
#include "Camera.h"
#include "CUDARenderer.cuh"

int main(void) {

    Film film(800, 600);

    Camera camera(
        Vector3(0, 0, 3),
        Vector3(0, 0, 0),
        Vector3(0, 1, 0),
        film.GetTamX(),
        film.GetTamY(),
        60);

    CUDARenderer* renderer = new CUDARenderer(film, camera, 0);

    bool rendering = true;
    while(rendering)
    {
        renderer->Render();
    }

    delete renderer;

    return 0;
}
