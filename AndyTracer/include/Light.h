#pragma once

#include "defs.h"
#include "Ray.h"

enum class LightType { Directional, Point };

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

    __host__ Light() :
	    type(LightType::Directional),
	    projectsShadow(false),
	    lightColor(GPUPixel(255)),
	    vec3LightColor(1.0f)
    {
    }

    __device__ Vector3 Shade(const Ray& ray, const ShapeIntersection& info) const
    {
        switch (type)
        {
        case LightType::Directional:
            return DirectionalShade(ray, info);
            break;
        case LightType::Point:
            return PointShade(ray, info);
            break;
        default:
			return Vector3(0.0f);
            break;
        }
    }

    __device__ Vector3 DirectionalShade(const Ray& ray, const ShapeIntersection& info) const
    {
        float intensity = fmaxf(0.0f, dot(info.normal, directionalData.direction));
        Vector3 diffuseColor = intensity * vec3LightColor;

        //el vector de la superficie a la camara
        Vector3 view = -ray.direction;
        //huh? ns esta en las diapos
        Vector3 r = 2 * dot(info.normal, directionalData.direction) * info.normal - directionalData.direction;

        float specular = fmaxf(0.0f, dot(view, r));
        //specular = powf(specular, 32.0f);
        float s2 = specular * specular;   // ^2
        float s4 = s2 * s2;               // ^4
        float s8 = s4 * s4;               // ^8
        float s16 = s8 * s8;              // ^16
        float specular32 = s16 * s16;     // ^32
        Vector3 specularColor = vec3LightColor * specular32;

        Vector3 finalColor = info.material.color * diffuseColor + specularColor;

        return Vector3(fminf(1.0f, finalColor.x), fminf(1.0f, finalColor.y), fminf(1.0f, finalColor.z));
    }

    __device__ Vector3 PointShade(const Ray& ray, const ShapeIntersection& info) const
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
        //specular = powf(specular, 32.0f);
        float s2 = specular * specular;   // ^2
        float s4 = s2 * s2;               // ^4
        float s8 = s4 * s4;               // ^8
        float s16 = s8 * s8;              // ^16
        float specular32 = s16 * s16;     // ^32

        Vector3 specularColor = vec3LightColor * specular32;

        Vector3 finalColor = info.material.color * diffuseColor + specularColor;

        return Vector3(fminf(1.0f, finalColor.x), fminf(1.0f, finalColor.y), fminf(1.0f, finalColor.z));
    }

    __device__ Vector3 GetShadowDir(const Vector3& p)
    {
        if(type == LightType::Point)
        {
            return normalize(pointData.position - p);
        }
    }
};

__host__ inline Light CreateDirectionalLight(const Vector3& dir, const GPUPixel& c)
{
    Light l;

    l.type = LightType::Directional;
    l.projectsShadow = false;
    l.directionalData.direction = dir;
    l.lightColor = c;
    l.vec3LightColor = Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);

    return l;
}

__host__ inline Light CreatePointLight(const Vector3& p, const GPUPixel& c)
{
    Light l;

    l.type = LightType::Point;
    l.projectsShadow = true;
    l.pointData.position = p;
    l.lightColor = c;
    l.vec3LightColor = Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);

    return l;
}