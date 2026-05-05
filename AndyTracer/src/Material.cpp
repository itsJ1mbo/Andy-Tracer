#include "Material.h"

Material::Material(std::shared_ptr<Texture> tex, float rf) : texture(tex), reflexFactor(rf)
{
}
