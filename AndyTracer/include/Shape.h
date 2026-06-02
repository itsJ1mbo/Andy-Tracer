#pragma once

#include "defs.h"
#include "Ray.h"

enum class ShapeType { Sphere, Quad };

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
    Material mat;

    union
    {
        SphereData sphereData;
        QuadData quadData;
    };

    __host__ Shape() : type()
    {
    }

    __device__ bool Intersect(const Ray& r, float tMin, float tMax, ShapeIntersection& info) const
    {
        switch (type)
        {
            case ShapeType::Sphere:
                return SphereIntersect(r, tMin, tMax, info);
                break;
            case ShapeType::Quad:
                return QuadIntersect(r, tMin, tMax, info);
                break;
            default:
                return false;
                break;
        }
    }

    __device__ bool SphereIntersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const
    {
        Vector3 oc = position - ray.origin;
        auto a = dot(ray.direction, ray.direction);
        auto b = dot(ray.direction, oc);
        auto c = dot(oc, oc) - sphereData.radius * sphereData.radius;
        auto discriminant = b * b - a * c;

        if (discriminant < 0)
            return false;

        auto sqrtd = sqrtf(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (b - sqrtd) / a;
        if (root <= tMin || tMax <= root) {
            root = (b + sqrtd) / a;
            if (root <= tMin || tMax <= root)
                return false;
        }

        info.position = ray.At(root);
        info.normal = normalize((info.position - position));
		info.material = mat;
        info.t = root;

        return true;
    }

    __device__ bool QuadIntersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const
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

        Vector3 unnormalized_normal = cross(quadData.width, quadData.height);
        Vector3 w = unnormalized_normal / dot(unnormalized_normal, unnormalized_normal);

        auto alpha = dot(w, cross(planar_hitpt_vector, quadData.height));
        auto beta = dot(w, cross(quadData.width, planar_hitpt_vector));
        if (alpha < 0.0f || alpha > 1.0f || beta < 0.0f || beta > 1.0f)
            return false;

        info.position = intersection;
        info.normal = quadData.normal;
		info.material = mat;
		info.t = t;


        return true;
    }

    __device__ AABB GetAABB() const
    {
        AABB box;

        if (type == ShapeType::Sphere)
        {
            box.bMin = position - Vector3(sphereData.radius);
            box.bMax = position + Vector3(sphereData.radius);
        }
        else if (type == ShapeType::Quad)
        {
			// Las esquinas del cuadrado
            Vector3 v0 = position;
            Vector3 v1 = position + quadData.width;
            Vector3 v2 = position + quadData.height;
            Vector3 v3 = position + quadData.width + quadData.height;

            // Punto minimo
            box.bMin.x = fminf(fminf(v0.x, v1.x), fminf(v2.x, v3.x));
            box.bMin.y = fminf(fminf(v0.y, v1.y), fminf(v2.y, v3.y));
            box.bMin.z = fminf(fminf(v0.z, v1.z), fminf(v2.z, v3.z));

            // Punto maximo
            box.bMax.x = fmaxf(fmaxf(v0.x, v1.x), fmaxf(v2.x, v3.x));
            box.bMax.y = fmaxf(fmaxf(v0.y, v1.y), fmaxf(v2.y, v3.y));
            box.bMax.z = fmaxf(fmaxf(v0.z, v1.z), fmaxf(v2.z, v3.z));

            float epsilon = 0.0001f;
            if (fabsf(box.bMax.x - box.bMin.x) < epsilon) { box.bMin.x -= epsilon; box.bMax.x += epsilon; }
            if (fabsf(box.bMax.y - box.bMin.y) < epsilon) { box.bMin.y -= epsilon; box.bMax.y += epsilon; }
            if (fabsf(box.bMax.z - box.bMin.z) < epsilon) { box.bMin.z -= epsilon; box.bMax.z += epsilon; }
        }
        
        return box;
    }
};

__host__ inline Shape CreateSphere(const Vector3& p, float r, const Material& m)
{
    Shape sh;
    sh.position = p;
    sh.mat = m;

    sh.type = ShapeType::Sphere;

    sh.sphereData.radius = r;

    return sh;
}

__host__ inline Shape CreateQuad(const Vector3& p, const Vector3& w, const Vector3& h, const Material& m)
{
    Shape sh;
    sh.position = p;
    sh.mat = m; 

    sh.type = ShapeType::Quad; 

    sh.quadData.width = w;
    sh.quadData.height = h;
    sh.quadData.normal = normalize(cross(h, w));

    return sh;
}
