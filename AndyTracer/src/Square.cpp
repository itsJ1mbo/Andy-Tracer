#include "Square.h"
#include "Ray.h"

Square::Square(const glm::vec3 p, const glm::vec3 h, const glm::vec3 w, const std::shared_ptr<Material>& m) : corner(p), width(w), height(h) 
{
	material = m;

    normal = glm::normalize(glm::cross(h, w));
}

bool Square::Intersect(const Ray& ray, float tMin, float tMax) const
{
    auto denom = dot(normal, ray.Direction());

    // No hit if the ray is parallel to the plane.
    if (std::fabs(denom) < 1e-8)
        return false;

    // Return false if the hit point parameter t is outside the ray interval.
    auto t = (glm::dot(normal, corner) - glm::dot(normal, ray.Origin())) / denom;
    if (t <= tMin || t >= tMax)
        return false;

    return true;
}

bool Square::Intersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const
{
    auto denom = dot(normal, ray.Direction());

    // No hit if the ray is parallel to the plane.
    if (std::fabs(denom) < 1e-8)
        return false;

    // Return false if the hit point parameter t is outside the ray interval.
    auto t = (glm::dot(normal, corner) - glm::dot(normal, ray.Origin())) / denom;
    if (t <= tMin || t >= tMax)
        return false;

    auto intersection = ray.At(t);
    // Esto sale en el libro pero no parece que cambie nada
    glm::vec3 planarHitPtVector = intersection - corner;
    auto w = normal / glm::dot(normal, normal);
    auto alpha = dot(w, cross(planarHitPtVector, height));
    auto beta = dot(w, cross(width, planarHitPtVector));
    if ((alpha <= 0.0f && alpha >= 1.0f) || (beta <= 0.0f && beta >= 1.0f))
        return false;

    info.position = intersection;
    info.material = material;
    info.normal = normal;

    info.uv.x = alpha;
    info.uv.y = beta;

    return true;
}
