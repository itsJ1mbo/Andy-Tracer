#include "Material.h"

Material::Material(const std::shared_ptr<Texture>& tex, float rf) : texture(tex), reflexFactor(rf)
{
}
