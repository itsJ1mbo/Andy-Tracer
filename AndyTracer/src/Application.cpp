#include "Application.h"

#include <algorithm>
#include <chrono>

#include "Film.h"
#include "Camera.h"
#include "CUDARenderer.cuh"
#include "Scene.h"
#include "Light.h"
#include "Shape.h"
#include "glm/vec3.hpp"
#include <glm/gtc/quaternion.hpp>

Application::Application()
{
    film = new Film(800, 600);

    film->GetWindow()->listener(this);

    camera = new Camera(
        Vector3(0, 0, 3),
        Vector3(0, 0, -1),
        Vector3(0, 1, 0),
        1.0f,
        film->GetTamX(),
        film->GetTamY(),
        60);

    // Materiales
    Material rojo = Material(Vector3(1.0f, 0.0f, 0.0f), 0.5f);
	Material verde = Material(Vector3(0.0f, 1.0f, 0.0f), 0.0f);
	Material azul = Material(Vector3(0.0f, 0.0f, 1.0f), 0.25f);
	Material amarillo = Material(Vector3(1.0f, 1.0f, 0.0f), 0.75f);

    //creamos las shapes
    std::vector<Shape> shapes;

    srand(static_cast<unsigned int>(time(nullptr)));
    Material materiales[] = { rojo, verde, azul };
    const int NUM_ESFERAS = 500;
    // Generacion masiva de esferas de forma aleatoria para poner a prueba el BVH
    for (int i = 0; i < NUM_ESFERAS; ++i)
    {
        float x = -20.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 40.0f);
        float y = 0.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 8.0f);
        float z = -5.0f - static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 40.0f);
        float radio = 0.1f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 0.5f);

        Material mat = materiales[rand() % 3];

        Shape esfera = CreateSphere(Vector3(x, y, z), radio, mat);
        shapes.push_back(esfera);
    }

    Shape suelo = CreateQuad(Vector3(50, -1, -50), Vector3(-100, 0, 0), Vector3(0, 0, 100), amarillo);
    shapes.push_back(suelo);

    // Nodos
    std::vector<BVHNode> cpuBVHNodes;

    BuildBVH(shapes, cpuBVHNodes, 0, shapes.size());

    //creamos las luces
    std::vector<Light> lights;
    Light dirLight = CreateDirectionalLight(Vector3(1, 1, 0), GPUPixel(255));
    lights.push_back(dirLight);
    Light pointLight = CreatePointLight(Vector3(2, 2, 0), GPUPixel(255));
    lights.push_back(pointLight);
    Light pointLight2 = CreatePointLight(Vector3(-2, 2, -3), GPUPixel(255));
    lights.push_back(pointLight2);

    //copiamos a la escena
    Scene escena;
    escena.shapes = shapes.data();
    escena.shapeCount = (int)shapes.size();
    escena.lights = lights.data();
    escena.lightCount = (int)lights.size();
	escena.bvhNodes = cpuBVHNodes.data();
	escena.nodesCount = (int)cpuBVHNodes.size();

    //creamos renderer
    renderer = new CUDARenderer(film, camera, &escena);
}

void Application::run()
{
	// Cosas de fps que he hecho para comprobar la mejora del BVH
    auto lastTime = std::chrono::high_resolution_clock::now();
    double timeAccumulator = 0.0;
    int frameCount = 0;

    // Bucle
    while (film->GetWindow()->open())
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        timeAccumulator += deltaTime.count();
        frameCount++;

        renderer->Render();

        if (timeAccumulator >= 1.0)
        {
            double fps = frameCount / timeAccumulator;
            double msPerFrame = (timeAccumulator / frameCount) * 1000.0;

            printf("FPS: %.2f | Frame Time: %.2f ms\n", fps, msPerFrame);

            frameCount = 0;
            timeAccumulator = 0.0;
        }
    }
}

void Application::free()
{
    delete film;
    delete camera;
    delete renderer;
}

int Application::BuildBVH(std::vector<Shape>& shapes, std::vector<BVHNode>& cpuBVHNodes, int start, int end)
{
    int nodeIdx = static_cast<int>(cpuBVHNodes.size());
    cpuBVHNodes.emplace_back();

	// AABB que envuelve a todas los objetos del nodo
    AABB box = shapes[start].GetAABB();
    for (int i = start + 1; i < end; i++) 
    {
        box = AABB::SurroundingBox(box, shapes[i].GetAABB());
    }
    cpuBVHNodes[nodeIdx].bounds = box;

    int count = end - start;
    if (count <= 2) 
    {
        // Si el nodo es hoja
        cpuBVHNodes[nodeIdx].isLeaf = true;
        cpuBVHNodes[nodeIdx].firstShape = start;
        cpuBVHNodes[nodeIdx].rightChild = count; 
    }
    else 
    {
        // Si es rama dividimos en dos
        cpuBVHNodes[nodeIdx].isLeaf = false;
        int mid = start + count / 2; // Punto medio

        // Elegimos un eje para cortar por la mitad
        // Para mas optimizacion se podria elegir un eje concreto en base a X pero me da pereza y pa esto asi vale
        int axis = rand() % 3;

        std::sort(shapes.begin() + start, shapes.begin() + end, [axis](const Shape& a, const Shape& b) {
            float centerA = (axis == 0) ? a.position.x : (axis == 1 ? a.position.y : a.position.z);
            float centerB = (axis == 0) ? b.position.x : (axis == 1 ? b.position.y : b.position.z);
            return centerA < centerB;
        });

        // Construimos los hijos recursivamente
        cpuBVHNodes[nodeIdx].leftChild = BuildBVH(shapes, cpuBVHNodes, start, mid);
        cpuBVHNodes[nodeIdx].rightChild = BuildBVH(shapes, cpuBVHNodes, mid, end);
    }

    return nodeIdx;
}

void Application::onKeyDown(DisplayInterface& display, Key key)
{
    printf("onKeyDown: key=%s\n", getKeyString(key));
}

void Application::onKeyPressed(DisplayInterface& display, Key key)
{
    printf("onKeyPressed: key=%s\n", getKeyString(key));

    Vector3 movementDir(0);
    float degrees = 0.0f;
    float movementSpeed = 0.1f;
    float rotationSpeed = 2.0f;

    //calculamos el vector right para movernos a los laterales
    Vector3 right = cross(camera->up, camera->forward);

    switch (key)
    {
    case Key::W:
        movementDir = -camera->forward;
        break;
    case Key::S:
        movementDir = camera->forward;
        break;
    case Key::A:
        movementDir = -right;
        break;
    case Key::D:
        movementDir = right;
        break;
    case Key::Left:
        degrees = rotationSpeed;
        break;
    case Key::Right:
        degrees = -rotationSpeed;
        break;
    }

    Vector3 newPos = camera->position + (movementDir * movementSpeed);

    Vector3 newForward = camera->forward;
    if (degrees != 0.0f)
    {
        glm::vec3 forwardOriginal(camera->forward.x, camera->forward.y, camera->forward.z);
        glm::vec3 rotAxis(camera->up.x, camera->up.y, camera->up.z);

        float rad = glm::radians(degrees);
        glm::quat rotQuaternion = glm::angleAxis(rad, glm::normalize(rotAxis));

        glm::vec3 forwardFinal = rotQuaternion * forwardOriginal;
        newForward = Vector3(forwardFinal.x, forwardFinal.y, forwardFinal.z);
    }

    camera->MoveCamera(newPos, newForward, camera->up);
}

void Application::onKeyUp(DisplayInterface& display, Key key)
{
    printf("onKeyUp: key=%s\n", getKeyString(key));
}

void Application::onMouseButtonDown(DisplayInterface& display, Mouse mouse)
{
    printf("onMouseButtonDown: buttons=%d,%d,%d x=%f, y=%f\n",
        mouse.buttons.left,
        mouse.buttons.middle,
        mouse.buttons.right,
        mouse.x,
        mouse.y);
}

void Application::onMouseButtonUp(DisplayInterface& display, Mouse mouse)
{
    printf("onMouseButtonUp: buttons=%d,%d,%d x=%f, y=%f\n",
        mouse.buttons.left,
        mouse.buttons.middle,
        mouse.buttons.right,
        mouse.x,
        mouse.y);
}

void Application::onMouseMove(DisplayInterface& display, Mouse mouse)
{
    printf("onMouseMove: buttons=%d,%d,%d x=%f, y=%f\n",
        mouse.buttons.left,
        mouse.buttons.middle,
        mouse.buttons.right,
        mouse.x,
        mouse.y);
}

void Application::onActivate(DisplayInterface& display, bool active)
{
    printf("onActivate: active=%d\n", active);
}

void Application::onOpen(DisplayInterface& display)
{
    printf("onOpen: \"%s\", %d x %d ", display.title(), display.width(), display.height());
    switch (display.mode())
    {
    case Mode::TrueColor: printf("truecolor"); break;
    case Mode::FloatingPoint: printf("floating point"); break;
    }
    switch (display.output())
    {
    case Output::Windowed: printf(" (windowed)\n"); break;
    case Output::Fullscreen: printf(" (fullscreen)\n"); break;
    default: break;
    }
}

bool Application::onClose(DisplayInterface& display)
{
    printf("onClose");
    return true;
}

const char* Application::getKeyString(Key key)
{
    switch (key)
    {
    case Key::Enter: return "Enter";
    case Key::BackSpace: return "BackSpace";
    case Key::Tab: return "Tab";
    case Key::Cancel: return "Cancel";
    case Key::Clear: return "Clear";
    case Key::Shift: return "Shift";
    case Key::Control: return "Control";
    case Key::Alt: return "Alt";
    case Key::Pause: return "Pause";
    case Key::CapsLock: return "CapsLock";
    case Key::Escape: return "Escape";
    case Key::Space: return "Space";
    case Key::PageUp: return "PageUp";
    case Key::PageDown: return "PageDown";
    case Key::End: return "End";
    case Key::Home: return "Home";
    case Key::Left: return "Left";
    case Key::Up: return "Up";
    case Key::Right: return "Right";
    case Key::Down: return "Down";
    case Key::Comma: return "Comma";
    case Key::Period: return "Period";
    case Key::Slash: return "Slash";
    case Key::Zero: return "Zero";
    case Key::One: return "One";
    case Key::Two: return "Two";
    case Key::Three: return "Three";
    case Key::Four: return "Four";
    case Key::Five: return "Five";
    case Key::Six: return "Six";
    case Key::Seven: return "Seven";
    case Key::Eight: return "Eight";
    case Key::Nine: return "Nine";
    case Key::SemiColon: return "SemiColon";
    case Key::Equals: return "Equals";
    case Key::A: return "A";
    case Key::B: return "B";
    case Key::C: return "C";
    case Key::D: return "D";
    case Key::E: return "E";
    case Key::F: return "F";
    case Key::G: return "G";
    case Key::H: return "H";
    case Key::I: return "I";
    case Key::J: return "J";
    case Key::K: return "K";
    case Key::L: return "L";
    case Key::M: return "M";
    case Key::N: return "N";
    case Key::O: return "O";
    case Key::P: return "P";
    case Key::Q: return "Q";
    case Key::R: return "R";
    case Key::S: return "S";
    case Key::T: return "T";
    case Key::U: return "U";
    case Key::V: return "V";
    case Key::W: return "W";
    case Key::X: return "X";
    case Key::Y: return "Y";
    case Key::Z: return "Z";
    case Key::OpenBracket: return "OpenBracket";
    case Key::BackSlash: return "BackSlash";
    case Key::CloseBracket: return "CloseBracket";
    case Key::NumPad0: return "NumPad0";
    case Key::NumPad1: return "NumPad1";
    case Key::NumPad2: return "NumPad2";
    case Key::NumPad3: return "NumPad3";
    case Key::NumPad4: return "NumPad4";
    case Key::NumPad5: return "NumPad5";
    case Key::NumPad6: return "NumPad6";
    case Key::NumPad7: return "NumPad7";
    case Key::NumPad8: return "NumPad8";
    case Key::NumPad9: return "NumPad9";
    case Key::Multiply: return "Multiply";
    case Key::Add: return "Add";
    case Key::Separator: return "Separator";
    case Key::Subtract: return "Subtract";
    case Key::Decimal: return "Decimal";
    case Key::Divide: return "Divide";
    case Key::F1: return "F1";
    case Key::F2: return "F2";
    case Key::F3: return "F3";
    case Key::F4: return "F4";
    case Key::F5: return "F5";
    case Key::F6: return "F6";
    case Key::F7: return "F7";
    case Key::F8: return "F8";
    case Key::F9: return "F9";
    case Key::F10: return "F10";
    case Key::F11: return "F11";
    case Key::F12: return "F12";
    case Key::Delete: return "Delete";
    case Key::NumLock: return "NumLock";
    case Key::ScrollLock: return "ScrollLock";
    case Key::PrintScreen: return "PrintScreen";
    case Key::Insert: return "Insert";
    case Key::Help: return "Help";
    case Key::Meta: return "Meta";
    case Key::BackQuote: return "BackQuote";
    case Key::Quote: return "Quote";
    case Key::Final: return "Final";
    case Key::Convert: return "Convert";
    case Key::NonConvert: return "NonConvert";
    case Key::Accept: return "Accept";
    case Key::ModeChange: return "ModeChange";
    case Key::Kana: return "Kana";
    case Key::Kanji: return "Kanji";
    default: return "Undefined";
    }
}
