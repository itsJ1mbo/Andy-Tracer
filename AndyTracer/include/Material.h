#pragma once
#include "Texture.h"
#include "defs.h"
#include <memory>

class Material
{
public:
    Material(std::shared_ptr<Texture> tex, float rf = 0.0f);

    inline Color GetColor(ShapeIntersection info) { return texture->GetTextureColor(info.uv.x, info.uv.y); }
    inline float GetReflexFactor() { return reflexFactor; }

private:
    std::shared_ptr<Texture> texture;
    float reflexFactor;
};

