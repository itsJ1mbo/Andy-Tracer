#include "Renderer.h"
#include "World.h"
#include "Scene.h"
#include "Light.h"
#include "glm/geometric.hpp"
#include <random>

Renderer::Renderer(const Film& f, const Camera& c, World* w, int r, int s) : film(f), camera(c), world(w), reflexes(r), samples(s)
{
}

Renderer::~Renderer()
{
    delete world;
}

void Renderer::Render()
{
    for (std::size_t y = 0; y < film.GetTamY(); ++y) {
        for (std::size_t x = 0; x < film.GetTamX(); ++x) {
            Color c = BLACK;
            for (int s = 0; s < samples; s++) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::normal_distribution<float> distrib(0, 1);

                const Ray ray_primary = camera.GetRay(x + distrib(gen), y + distrib(gen));
                int reflexes = this->reflexes;
                c += RayColor(ray_primary, reflexes);
            }
            c /= samples;
            film.AddPixel(c);
        }
    }
}

Color Renderer::RayColor(const Ray& r, int& reflexes)
{
    ShapeIntersection info;
    if (world->GetScene()->Intersect(r, 0, INFINITY, info)) {
        return Shade(r, info, reflexes);
    }

   /*glm::vec3 unit_direction = glm::normalize(r.direction());
    float a = 0.5 * (unit_direction.y + 1.0);
    return (1.0f - a) * Color(1.0, 1.0, 1.0) + a * Color(0.5, 0.7, 1.0);*/
    return Color(0, 0, 0);
}

Color Renderer::Shade(const Ray& ray, ShapeIntersection& info, int& reflexes)
{
    Color ret = BLACK;

    for (auto l : world->GetLights()) {
        if (l->ProjectsShadows()) {
            glm::vec3 dir = l->ShadowDir(info.position);
            auto shadowRay = Ray(info.position + dir * rayEpsilon, dir);
            ShapeIntersection shadowInfo;
            if (world->GetScene()->Intersect(shadowRay, 0, INFINITY, shadowInfo)) {
                continue;
            }
        }
        ret += l->Shade(ray, info);
    }

    if (info.material->GetReflexFactor() > 0 && reflexes > 0) {
        reflexes--;
        auto dir = Reflect(info);
        Ray reflejo = Ray(info.position + dir * rayEpsilon, dir);
        ret += info.material->GetReflexFactor() * RayColor(reflejo, reflexes);
    }
    // Ambiente
    ret += Color(0.2, 0.2, 0.2) * info.material->GetColor(info);

    return ret;
}

glm::vec3 Renderer::Reflect(ShapeIntersection& info)
{
    glm::vec3 dir = glm::normalize(info.position - camera.GetPos());
    return glm::normalize(dir - (2 * glm::dot(dir, info.normal)) * info.normal);
}
