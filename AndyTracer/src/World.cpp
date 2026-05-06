#include "World.h"
#include "Scene.h"
#include "Light.h"

World::~World()
{
    delete things;
    things = nullptr;

    for (auto c : lights) {
        delete c;
        c = nullptr;
    }
}
