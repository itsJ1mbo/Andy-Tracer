#pragma once

#include "Texture.h"
#include <memory>

class CheckerTexture : public Texture
{
public:
    CheckerTexture(const std::shared_ptr<Texture>& t1, const std::shared_ptr<Texture>& t2, int f, int c) : texture1(t1), texture2(t2), fils(f), cols(c) {};

    Color GetTextureColor(float u, float v) const override;

private:
    std::shared_ptr<Texture> texture1;
    std::shared_ptr<Texture> texture2;
    int fils;
    int cols;
};

