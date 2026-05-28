#pragma once
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <memory>

class Material;

struct ShapeIntersection {
    std::shared_ptr<Material> material;
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

using integer8 = unsigned char;

struct GPUPixel
{
    integer8 b, g, r, a;
};
