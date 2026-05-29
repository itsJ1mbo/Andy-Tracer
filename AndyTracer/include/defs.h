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
    return sqrtf(length_squared(v));
}

__host__ __device__ inline Vector3 normalize(const Vector3& v) {
    float len = length(v);
    if (len == 0.0f) return Vector3(0.0f, 0.0f, 0.0f);
    return v * (1.0f / len);
}
