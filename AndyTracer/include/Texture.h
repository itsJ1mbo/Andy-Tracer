#pragma once

#include "Color.h"

class Texture
{
public: 
    virtual ~Texture() = default;
    virtual Color GetTextureColor(float u, float v) const = 0;
};

