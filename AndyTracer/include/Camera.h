#pragma once
#include "defs.h"
#include "Ray.h"

struct Camera
{
    Vector3 position;
    Vector3 delta_x;
    Vector3 delta_y;
    Vector3 position_top_left;

    __host__ Camera(
        Vector3 p,
        Vector3 look,
        Vector3 up,
        int width,
        int height,
        const float fov_degrees_vertical
    )
    {
        position = p;
        const float fov_radians_vertical = (fov_degrees_vertical * 0.5f) * PI / 180.0f;
        const float half_height_normalized = tan(fov_radians_vertical);

        const Vector3 forward_displacement = position - look;
        const float focal_length = length(forward_displacement);
        const Vector3 forward = forward_displacement / focal_length;
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
    }

    __device__ inline Ray GetRay(int x, int y)
    {
        const Vector3 sample =
            position_top_left + delta_x * (float)x + delta_y * (float)y;
        const Vector3 displacement = (sample - position);

        return Ray{ position, normalize(displacement) };
    }

};
