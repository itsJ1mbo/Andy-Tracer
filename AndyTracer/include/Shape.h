#pragma once
#include "defs.h"
#include "Ray.h"

enum ShapeType { Sphere, Quad };

struct SphereData
{
    float radius;
};

struct QuadData
{
    Vector3 width;
    Vector3 height;
    Vector3 normal;
};

struct Shape
{
    ShapeType type;
    Vector3 position;

    union
    {
        SphereData sphereData;
        QuadData quadData;
    };

    __host__ Shape() {}

    __device__ bool Intersect(Ray r, float tMin, float tMax, ShapeIntersection& info)
    {
        switch (type)
        {
            case Sphere:
                return SphereIntersect(r, tMin, tMax, info);
                break;
            case Quad:
                return QuadIntersect(r, tMin, tMax, info);
                break;
        }
    }

    __device__ bool SphereIntersect(Ray ray, float tMin, float tMax, ShapeIntersection& info)
    {
        Vector3 oc = position - ray.origin;
        auto a = pow(length(ray.direction), 2);
        auto b = dot(ray.direction, oc);
        auto c = pow(length(oc), 2) - sphereData.radius * sphereData.radius;
        auto discriminant = b * b - a * c;

        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (b - sqrtd) / a;
        if (root <= tMin || tMax <= root) {
            root = (b + sqrtd) / a;
            if (root <= tMin || tMax <= root)
                return false;
        }

        info.position = ray.At(root);
        info.normal = normalize((info.position - position));

        return true;
    }

    __device__ bool QuadIntersect(Ray ray, float tMin, float tMax, ShapeIntersection& info)
    {
        float denom = dot(quadData.normal, ray.direction);

        // No hit if the ray is parallel to the plane.
        if (fabsf(denom) < 1e-8)
            return false;

        // Return false if the hit point parameter t is outside the ray interval.
        float t = (dot(quadData.normal, position) - dot(quadData.normal, ray.origin)) / denom;
        if (t <= tMin || t >= tMax)
            return false;

        auto intersection = ray.At(t);
        // Esto sale en el libro pero no parece que cambie nada
        Vector3 planar_hitpt_vector = intersection - position;
        auto w = quadData.normal / dot(quadData.normal, quadData.normal);
        auto alpha = dot(w, cross(planar_hitpt_vector, quadData.height));
        auto beta = dot(w, cross(quadData.width, planar_hitpt_vector));
        if ((alpha <= 0 && alpha >= 1) || (beta <= 0 && beta >= 1))
            return false;

        info.position = intersection;
        info.normal = quadData.normal;


        return true;
    }
};

__host__ inline Shape CreateSphere(Vector3 p, float r)
{
    Shape sh;
    sh.position = p;

    sh.type = Sphere;

    sh.sphereData.radius = r;

    return sh;
}

__host__ inline Shape CreateQuad(Vector3 p, Vector3 w, Vector3 h)
{
    Shape sh;
    sh.position = p;

    sh.type = Quad;

    sh.quadData.width = w;
    sh.quadData.height = h;
    sh.quadData.normal = normalize(cross(h, w));

    return sh;
}
