#pragma once
#include <iosfwd>
#include <vector>
#include <PixelToaster/include/PixelToaster.h>

#include "AABB.h"
#include "Shape.h"

struct Film;
struct Camera;
struct CUDARenderer;

using namespace PixelToaster;

class Application : public Listener
{
public:
    Application();
    ~Application() = default;

    void run();
    void free();

    int BuildBVH(std::vector<Shape>& shapes, std::vector<BVHNode>& cpuBVHNodes, int start, int end);

private:
    virtual void onKeyDown(DisplayInterface& display, Key key) override;

    virtual void onKeyPressed(DisplayInterface& display, Key key) override;

    virtual void onKeyUp(DisplayInterface& display, Key key) override;

    virtual void onMouseButtonDown(DisplayInterface& display, Mouse mouse) override;

    virtual void onMouseButtonUp(DisplayInterface& display, Mouse mouse) override;

    virtual void onMouseMove(DisplayInterface& display, Mouse mouse) override;

    virtual void onActivate(DisplayInterface& display, bool active) override;

    virtual void onOpen(DisplayInterface& display) override;

    virtual bool onClose(DisplayInterface& display) override;

    const char* getKeyString(Key key);

    Film* film;
    Camera* camera;
    CUDARenderer* renderer;
};
