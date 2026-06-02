#pragma once

#include "PixelToaster/include/PixelToaster.h"

struct GPUPixel;

class Film 
{
public:
    Film(int x, int y);
    ~Film();

    int GetTamX() const { return tamX; }
    int GetTamY() const { return tamY; }
    float GetAspectRatio() const { return aspectRatio; }

    void Display();

    void CopyBuffer(GPUPixel* buffer);

    PixelToaster::Display* GetWindow() { return &window; }

private:
    int tamX;
    int tamY;

    float aspectRatio;

    PixelToaster::Display window;

    PixelToaster::TrueColorPixel* pixels;
};
