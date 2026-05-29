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
        auto denom = dot(quadData.normal, ray.direction);

        // No hit if the ray is parallel to the plane.
        if (std::fabs(denom) < 1e-8)
            return false;

        // Return false if the hit point parameter t is outside the ray interval.
        auto t = (dot(quadData.normal, position) - dot(quadData.normal, ray.origin)) / denom;
        if (t <= tMin || t >= tMax)
            return false;

        return true;
    }
};

__host__ inline Shape CreateSphere(Vector3 p, float r)
{
    Shape sh;
    sh.position = p;

    sh.type = Sphere;

    SphereData sd;
    sd.radius = r;
    sh.sphereData = sd;

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
