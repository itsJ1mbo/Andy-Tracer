#include "Sphere.h"
#include "Ray.h"
#include "glm/gtc/constants.hpp"

Sphere::Sphere(glm::vec3 c, float r, const std::shared_ptr<Material>& m) : center(c), radius(r) {
    material = m;
}

bool Sphere::Intersect(const Ray& ray, float tMin, float tMax) const
{
    glm::vec3 oc = ray.Origin() - center;
    auto a = glm::dot(ray.Direction(), ray.Direction());
    auto b = 2 * glm::dot(ray.Direction(), oc);
    auto c = glm::dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return false;

    auto sqrt = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    float root = (-b - sqrt) / (2.0f * a);
    if (root <= tMin || tMax <= root) {
        root = (b + sqrt) / a;
        if (root <= tMin || tMax <= root)
            return false;
    }

    return true;
}

bool Sphere::Intersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const
{
    glm::vec3 oc = center - ray.Origin();
    auto a = pow(glm::length(ray.Direction()), 2.0f);
    auto b = glm::dot(ray.Direction(), oc);
    auto c = pow(glm::length(oc), 2.0f) - radius * radius;
    auto discriminant = b * b - a * c;

    if (discriminant < 0)
        return false;

    auto sqrt = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (b - sqrt) / a;
    if (root <= tMin || tMax <= root) {
        root = (b + sqrt) / a;
        if (root <= tMin || tMax <= root)
            return false;
    }

    info.position = ray.At(root);
    info.normal = glm::normalize((info.position - center));

    info.material = material;

    auto theta = std::acos(-info.position.y);
    auto phi = std::atan2(-info.position.z, info.position.x) + glm::pi<float>();
    info.uv.x = phi / (2.0f * glm::pi<float>());
    info.uv.y = theta / glm::pi<float>();

    return true;    
}
