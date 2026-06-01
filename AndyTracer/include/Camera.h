#pragma once
#include "defs.h"
#include "Ray.h"

struct Camera
{
    Vector3 position;
    Vector3 delta_x;
    Vector3 delta_y;
    Vector3 position_top_left;
    Vector3 forward;
    Vector3 up;
    Vector3 desenfoqueU;
    Vector3 desenfoqueV;
    float focal_length;
    int width;
    int height;
    float fov_degrees_vertical;
    float blurAngle;
    float focalDist;

    __host__ Camera(
        Vector3 p,
        Vector3 forw,          
        Vector3 u,
        float fl,             
        int w,
        int h,
        const float fovDV,
        float bAngle = 0.0f,
        float fDist = 1.0f
    )
    {
        position = p;
        //este menos es necesario para que cuando creamos la camara y la escena todo siga el mismo convenio de ejes
        forward = -normalize(forw);
        up = u;
        focal_length = fl;
        width = w;
        height = h;
        fov_degrees_vertical = fovDV;

        blurAngle = bAngle;
        focalDist = fDist;

        CalculateViewport();
    }

    __device__ inline Ray GetRay(float x, float y, int seed) const
    {
        const Vector3 sample =
            position_top_left + delta_x * x + delta_y * y;

        const Vector3 p = GetRandomPos(seed);
        const Vector3 origen = position + p.x * desenfoqueU + p.y * desenfoqueV;

        const Vector3 displacement = (sample - origen);

        return Ray{ origen, normalize(displacement) };
    }

    __device__ inline Vector3 GetRandomPos(int seed) const
    {
        // No se puede usar <random>
        seed = 1664525 * seed + 1013904223;
        float r1 = static_cast<float>(seed & 0x00FFFFFF) / static_cast<float>(0x01000000);
        seed = 1664525 * seed + 1013904223;
        float r2 = static_cast<float>(seed & 0x00FFFFFF) / static_cast<float>(0x01000000);

        float theta = r1 * 2.0f * PI;
        float r = sqrtf(r2);

        return Vector3(r * cosf(theta), r * sinf(theta), 0.0f);
    }

    __host__ void CalculateViewport()
    {
        const float fov_radians_vertical = (fov_degrees_vertical * 0.5f) * PI / 180.0f;
        const float half_height_normalized = tan(fov_radians_vertical);

        const Vector3 right = cross(up, forward);
        const Vector3 v = cross(forward, right);

        const float half_height_viewport = focal_length * half_height_normalized;
        const float half_width_viewport = half_height_viewport * (float)width / (float)height;

        const float height_viewport = half_height_viewport * 2.0f;
        const float width_viewport = half_width_viewport * 2.0f;

        const float pixel_height = height_viewport / float(height);
        const float pixel_width = width_viewport / float(width);

        delta_x = right * pixel_width;
        delta_y = -v * pixel_height;

        position_top_left =
            position - focal_length * forward
            + v * half_height_viewport + delta_x * 0.5f
            - right * half_width_viewport + delta_y * 0.5f;

        float blur_radians = (blurAngle * PI / 180.0f);
        float radioDesenfoque = focalDist * tan(blur_radians / 2.0f);
        desenfoqueU = right * radioDesenfoque;
        desenfoqueV = v * radioDesenfoque;
    }

    __host__ void MoveCamera(const Vector3& newPos)
    {
        position = newPos;

        CalculateViewport();
    }

    __host__ void MoveCamera(const Vector3& newPos, const Vector3& newForward, const Vector3& newUp)
    {
        position = newPos;
        forward = newForward;
        up = newUp;

        CalculateViewport();
    }
};