#include "Film.h"
#include "Camera.h"
#include "Sphere.h"
//#include "Renderer.h"
#include "Scene.h"
#include "World.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Square.h"
#include "ConstantTexture.h"
#include "CheckerTexture.h"
#include "CUDARenderer.cuh"

#include <fstream>

int main(void) {
    std::ofstream outFile("imagen.ppm");

    Film film(800, 600);
    Camera camera(
        glm::vec3(0, 0, 3),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0),
        film,
        60,
        0.6f, 
        1);

    std::shared_ptr<Texture> texAzul = std::make_shared<ConstantTexture>(BLUE);
    std::shared_ptr<Texture> texAm = std::make_shared<ConstantTexture>(YELLOW);
    std::shared_ptr<Texture> texR = std::make_shared<ConstantTexture>(RED);
    std::shared_ptr<Texture> texV = std::make_shared<ConstantTexture>(GREEN);
    std::shared_ptr<Texture> texGris = std::make_shared<ConstantTexture>(Color(0.25, 0.25, 0.25));
    std::shared_ptr<Texture> patron1 = std::make_shared<CheckerTexture>(texGris, texAzul, 4, 4);
    std::shared_ptr<Texture> patron2 = std::make_shared<CheckerTexture>(std::make_shared<ConstantTexture>(Color(1.0f, 1.0f, 1.0f)), texAzul, 4, 4);
    std::shared_ptr<Texture> tex1 = std::make_shared<CheckerTexture>(patron1, texAzul, 4, 4);
    std::shared_ptr<Texture> tex2 = std::make_shared<CheckerTexture>(patron1, patron2, 3, 4);

    std::shared_ptr<Material> azul = std::make_shared<Material>(texAzul, 0.5);
    std::shared_ptr<Material> amarillo = std::make_shared<Material>(texAm, 0.5);
    std::shared_ptr<Material> rojo = std::make_shared<Material>(texR, 0.5);
    std::shared_ptr<Material> verde = std::make_shared<Material>(texV, 0.5);

    std::shared_ptr<Material> checker = std::make_shared<Material>(patron1, 0.5);

    Sphere* obj1 = new Sphere(glm::vec3(2, 0, -2), 1.0, azul);
    Sphere* obj2 = new Sphere(glm::vec3(0, 0, -4), 1.0, amarillo);
    Sphere* obj3 = new Sphere(glm::vec3(-2, 0, -6), 1.0, rojo);
    Sphere* obj4 = new Sphere(glm::vec3(0, -100, -2), 99, verde);

    Square* sqr1 = new Square(glm::vec3(3, -1, 5), glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), checker);

    Scene* scene = new Scene();
    scene->Add(obj1); 
    scene->Add(obj2); 
    scene->Add(obj3);
    scene->Add(sqr1);

    std::list<Light*> luces;
    DirectionalLight* dirLight = new DirectionalLight(glm::vec3(1, 1, 0), WHITE);
    PointLight* pointLight = new PointLight(glm::vec3(2, 2, 0), WHITE);
    PointLight* pointLight2 = new PointLight(glm::vec3(-2, 2, -3), WHITE);

    //luces.push_back(dirLight);
    luces.push_back(pointLight);
    luces.push_back(pointLight2);

    World* world = new World(scene, luces);

    //Renderer* renderer = new Renderer(film, camera, world, 0, 1);

    CUDARenderer* renderer = new CUDARenderer(film, camera, world, 0, 1);

    bool rendering = true;
    while(rendering)
    {
        renderer->Render();
    }

    delete renderer;

    return 0;
}
