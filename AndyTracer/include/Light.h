#pragma once
#pragma once
#include "defs.h"
#include "Ray.h"

enum LightType { Directional, Point };

struct DirectionalData
{
    Vector3 direction;
};

struct PointData
{
    Vector3 position;
};

struct Light
{
    LightType type;
    bool projectsShadow;
    GPUPixel lightColor;
    Vector3 vec3LightColor;

    union
    {
        DirectionalData directionalData;
        PointData pointData;
    };

    __host__ Light() : type(Directional), projectsShadow(false), lightColor(GPUPixel(255)), vec3LightColor(1.0f){}

    __device__ Vector3 Shade(Ray ray, ShapeIntersection& info)
    {
        switch (type)
        {
        case Directional:
            return DirectionalShade(ray, info);
            break;
        case Point:
            return PointShade(ray, info);
            break;
        }
    }

    __device__ Vector3 DirectionalShade(Ray ray, ShapeIntersection& info)
    {
        float intensity = fmaxf(0.0f, dot(info.normal, directionalData.direction));
        Vector3 diffuseColor = intensity * vec3LightColor;

        //el vector de la superficie a la camara
        Vector3 view = -ray.direction;
        //huh? ns esta en las diapos
        Vector3 r = 2 * dot(info.normal, directionalData.direction) * info.normal - directionalData.direction;

        float specular = fmaxf(0.0f, dot(view, r));
        specular = powf(specular, 32.0f);
        Vector3 specularColor = vec3LightColor * specular;

        Vector3 finalColor = Vector3(1.0f, 1.0f, 1.0f) * diffuseColor + specularColor;

        return Vector3(fminf(1.0f, finalColor.x), fminf(1.0f, finalColor.y), fminf(1.0f, finalColor.z));
    }

    __device__ Vector3 PointShade(Ray ray, ShapeIntersection& info)
    {
        Vector3 dir = normalize(info.position - pointData.position);

        float intensity = abs(dot(info.normal, dir));
        Vector3 diffuseColor = intensity * vec3LightColor;

        //el vector de la superficie a la camara
        Vector3 view = -ray.direction;
        //huh? ns esta en las diapos
        Vector3 r = 2 * dot(info.normal, dir) * info.normal - dir;
        //ray = glm::reflect(-dir, info.normal);

        float specular = fmaxf(0.0f, dot(view, r));
        specular = powf(specular, 32.0f);
        Vector3 specularColor = vec3LightColor * specular;

        Vector3 finalColor = Vector3(1.0f, 1.0f, 1.0f) * diffuseColor + specularColor;

        return Vector3(fminf(1.0f, finalColor.x), fminf(1.0f, finalColor.y), fminf(1.0f, finalColor.z));
    }

    __device__ Vector3 GetShadowDir(Vector3 p)
    {
        if(type == Point)
        {
            return normalize(pointData.position - p);
        }
    }
};

__host__ inline Light CreateDirectionalLight(Vector3 dir, GPUPixel c)
{
    Light l;

    l.type = Directional;
    l.projectsShadow = false;
    l.directionalData.direction = dir;
    l.lightColor = c;
    l.vec3LightColor = Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);

    return l;
}

__host__ inline Light CreatePointLight(Vector3 p, GPUPixel c)
{
    Light l;

    l.type = Point;
    l.projectsShadow = true;
    l.pointData.position = p;
    l.lightColor = c;
    l.vec3LightColor = Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);

    return l;
}