#pragma once
#include "Color.h"
class Texture
{
public:
    ~Texture() = default;
    virtual Color GetTextureColor(float u, float v) const = 0;
};

