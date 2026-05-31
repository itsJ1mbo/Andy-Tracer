#include "Application.h"
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
	Material verde = Material(Vector3(0.0f, 1.0f, 0.0f), 0.5f);
	Material azul = Material(Vector3(0.0f, 0.0f, 1.0f), 0.5f);
	Material amarillo = Material(Vector3(1.0f, 1.0f, 0.0f), 0.5f);

    //creamos las shapes
    std::vector<Shape> shapes;
    Shape esfera = CreateSphere(Vector3(2, 0, -2), 1, rojo);
    shapes.push_back(esfera);
    Shape esfera2 = CreateSphere(Vector3(0, 0, -4), 1, verde);
    shapes.push_back(esfera2);
    Shape esfera3 = CreateSphere(Vector3(-2, 0, -6), 1, azul);
    shapes.push_back(esfera3);
    Shape suelo = CreateQuad(Vector3(50, -1, -50), Vector3(-100, 0, 0), Vector3(0, 0, 100), amarillo);
    shapes.push_back(suelo);

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

    //creamos renderer
    renderer = new CUDARenderer(film, camera, &escena, 10);
}

void Application::run()
{
    while(film->GetWindow()->open())
    {
        renderer->Render();
    }
}

void Application::free()
{
    delete film;
    delete camera;
    delete renderer;
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
