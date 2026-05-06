#pragma once

#include "Texture.h"
#include "defs.h"
#include <memory>

class Material
{
public:
    Material(const std::shared_ptr<Texture>& tex, float rf = 0.0f);

    inline Color GetColor(const ShapeIntersection& info) const { return texture->GetTextureColor(info.uv.x, info.uv.y); }
    inline float GetReflexFactor() const { return reflexFactor; }

private:
    std::shared_ptr<Texture> texture;
    float reflexFactor;
};

