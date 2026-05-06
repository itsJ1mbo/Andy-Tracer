#include "PointLight.h"
#include "Shape.h"
#include "Ray.h"
#include "glm/geometric.hpp"

PointLight::PointLight(const glm::vec3& pos, Color c) : position(pos)
{
    lightColor = c;
}

Color PointLight::Shade(const Ray& ray, ShapeIntersection& info)
{
    glm::vec3 dir = glm::normalize(info.position - position);

    float intensity = abs(glm::dot(info.normal, dir));
    diffuseColor = intensity * lightColor;

    //el vector de la superficie a la camara
    glm::vec3 view = -ray.Direction();
    //huh? ns esta en las diapos
    glm::vec3 r = 2 * glm::dot(info.normal, dir) * info.normal - dir;
    //ray = glm::reflect(-dir, info.normal);

    float specular = glm::max(0.0f, glm::dot(view, r));
    specular = std::pow(specular, 128.0f);
    specularColor = lightColor * specular;

    return (info.material->GetColor(info) * diffuseColor) + specularColor;
}

bool PointLight::ProjectsShadows()
{
    //hacer cosas
    return true;
}

glm::vec3 PointLight::ShadowDir(const glm::vec3& p)
{
    return glm::normalize(position - p);
}
