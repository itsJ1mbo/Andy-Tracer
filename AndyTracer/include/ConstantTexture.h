#pragma once
#include "Texture.h"
class ConstantTexture : public Texture
{
public:
    ConstantTexture(Color c) : color(c) {}

    Color GetTextureColor(float u, float v) const override;
private:
    Color color;
};

