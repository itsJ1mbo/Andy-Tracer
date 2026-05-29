#pragma once
#include "cuda_runtime.h"
#include "cmath"

const float PI = 3.14159265359;

using integer8 = unsigned char;

struct GPUPixel
{
    integer8 b, g, r, a;

    __host__ __device__ GPUPixel(integer8 n)
    {
        r = n;
        g = n;
        b = n;
        a = n;
    }

    __host__ __device__ GPUPixel(integer8 red, integer8 green, integer8 blue, integer8 alpha)
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }
};

struct Vector3
{
    float x, y, z;

    __host__ __device__ Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

    __host__ __device__ Vector3(float a) : x(a), y(a), z(a) {}

    __host__ __device__ Vector3(float a, float b, float c) : x(a), y(b), z(c) {}
};

__host__ __device__ inline Vector3 operator-(const Vector3& v) {
    return Vector3(-v.x, -v.y, -v.z);
}

__host__ __device__ inline Vector3 operator+(const Vector3& u, const Vector3& v) {
    return Vector3(u.x + v.x, u.y + v.y, u.z + v.z);
}

__host__ __device__ inline Vector3 operator-(const Vector3& u, const Vector3& v) {
    return Vector3(u.x - v.x, u.y - v.y, u.z - v.z);
}

__host__ __device__ inline Vector3 operator*(float t, const Vector3& v) {
    return Vector3(t * v.x, t * v.y, t * v.z);
}

__host__ __device__ inline Vector3 operator*(const Vector3& v, float t) {
    return t * v;
}

__host__ __device__ inline Vector3 operator/(const Vector3& v, float t) {
    return Vector3(v.x / t, v.y / t, v.z / t);
}

__host__ __device__ inline float dot(const Vector3& u, const Vector3& v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

__host__ __device__ inline Vector3 cross(const Vector3& u, const Vector3& v) {
    return Vector3(u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x);
}

__host__ __device__ inline float length_squared(const Vector3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

__host__ __device__ inline float length(const Vector3& v) {
    return std::sqrt(length_squared(v));
}

__host__ __device__ inline Vector3 normalize(const Vector3& v) {
    float len = length(v);
    if (len == 0.0f) return Vector3(0, 0, 0);
    return v * (1.0f / len);
}

struct Ray
{
    Vector3 position;
    Vector3 direction;

    __device__ Ray(Vector3 pos,Vector3 dir)
    {
        position = pos;
        direction = dir;
    }

    __device__ Vector3 At(float t)
    {
        return position + t * direction;
    }
};

struct Camera
{
    Vector3 position;
    Vector3 delta_x;
    Vector3 delta_y;
    Vector3 position_top_left;

    Camera(
        Vector3 p,
        Vector3 look,
        Vector3 up,
        int width,
        int height,
        const float fov_degrees_vertical,
        float blurAngle,
        float focalDist
    )
    {
        position = p;
        const float fov_radians_vertical = (fov_degrees_vertical * 0.5) * PI / 180;
        const float half_height_normalized = tan(fov_radians_vertical);

        const Vector3 forward_displacement = position - look;
        const float focal_length = length(forward_displacement);
        const Vector3 forward = forward_displacement / focal_length;
        const Vector3 right = cross(up, forward);
        const Vector3 v = cross(forward, right);

        const float half_height_viewport = focal_length * half_height_normalized;
        const float half_width_viewport = half_height_viewport * (float)width / (float)height;

        const float height_viewport = half_height_viewport * 2.0;
        const float width_viewport = half_width_viewport * 2.0;

        const float pixel_height = height_viewport / float(height);
        const float pixel_width = width_viewport / float(width);

        delta_x = right * pixel_width;
        delta_y = -v * pixel_height;
        position_top_left =
            position - focal_length * forward
            + v * half_height_viewport + delta_x * 0.5f
            - right * half_width_viewport + delta_y * 0.5f;
    }

    __device__ Ray GetRay(int x, int y)
    {
        const Vector3 sample =
            position_top_left + delta_x * (float)x + delta_y * (float)y;
        const Vector3 displacement = (sample - position);

        return Ray{ position, normalize(displacement) };
    }

};
