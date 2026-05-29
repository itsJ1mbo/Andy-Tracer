#include "Film.h"
#include "CUDARenderer.cuh"

int main(void) {

    Film film(800, 600);

    CUDARenderer* renderer = new CUDARenderer(film, 0);

    bool rendering = true;
    while(rendering)
    {
        renderer->Render();
    }

    delete renderer;

    return 0;
}
