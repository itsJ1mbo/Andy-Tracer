#pragma once

#include <list>

class Scene;
class Light;

class World
{
public:
    World(Scene* c, const std::list<Light*>& oc) : things(c), lights(oc) {};
    ~World();
   
    inline Scene* GetScene() const { return things; }
    inline std::list<Light*> GetLights() const { return lights; }

private:
    Scene* things;
    std::list<Light*> lights;
};

