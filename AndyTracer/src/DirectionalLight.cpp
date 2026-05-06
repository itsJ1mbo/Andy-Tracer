#include "DirectionalLight.h"
#include "Shape.h"
#include "Ray.h"
#include "glm/geometric.hpp"

DirectionalLight::DirectionalLight(glm::vec3 dir, Color c) : direction(glm::normalize(dir)) 
{
    lightColor = c;
}

Color DirectionalLight::Shade(const Ray& ray, ShapeIntersection& info)
{
    float intensity = std::max(0.0f, glm::dot(info.normal, direction));
    diffuseColor = intensity * lightColor;

    //el vector de la superficie a la camara
    glm::vec3 view = -ray.Direction();
    //huh? ns esta en las diapos
    glm::vec3 r = 2 * glm::dot(info.normal, direction) * info.normal - direction;
    //r = glm::reflect(-direction, info.normal);

    float specular = glm::max(0.0f, glm::dot(view, r));
    specular = std::pow(specular, 10.0f);
    specularColor = lightColor * specular;

    return info.material->GetColor(info) * diffuseColor + specularColor;
}
