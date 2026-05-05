#pragma once
#include <list>

class Scene;
class Light;

class World
{
public:
    World(Scene* c, const std::list<Light*>& oc) : cosas(c), otrasCosas(oc) {};
    inline ~World() {
        delete cosas;
        cosas = nullptr;

        for (auto c : otrasCosas) {
            delete c;
            c = nullptr;
        }

    }
   
    inline Scene* GetScene() { return cosas; }

    inline std::list<Light*> GetLights() const { return otrasCosas; }

private:
    Scene* cosas;
    std::list<Light*> otrasCosas;
};

