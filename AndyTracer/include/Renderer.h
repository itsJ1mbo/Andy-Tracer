#pragma once

#include "Film.h"
#include "Camera.h"

class World;
struct ShapeIntersection;

class Renderer
{
public:
    Renderer(const Film& f, const Camera& c, World* w, int r, int s);

    ~Renderer();

    void Render();

private:
    Color RayColor(const Ray& r, int& ref);

    Color Shade(const Ray& ray, ShapeIntersection& info, int& ref);

    glm::vec3 Reflect(const ShapeIntersection& info) const;

    const Camera camera;
    Film film; 
    World* world;
    int reflexes;
    int samples;

    const float rayEpsilon = 0.001f;
};

