#pragma once
//#include "Film.h"
//#include "Camera.h"
//
//class World;
//struct ShapeIntersection;

class CUDARenderer
{
public:
    //CUDARenderer(const Film& f, const Camera& c, World* w, int r, int s);
    CUDARenderer(int r, int s);

    ~CUDARenderer();

    void Render();

private:
    //const Camera camera;
    //Film film;
    //World* world;
    int reflexes;
    int samples;

    const float rayEpsilon = 0.001f;
};