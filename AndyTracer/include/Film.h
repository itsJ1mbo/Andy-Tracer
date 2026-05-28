#ifndef _FILM_H
#define _FILM_H

#include "PixelToaster/include/PixelToaster.h"
#include <iostream>
#include "Color.h"
#include "defs.h"

class Film 
{
public:
    Film(int x, int y);

    void AddPixel(Color color, int x, int y);

    int GetTamX() const { return tamX; }
    int GetTamY() const { return tamY; }
    float GetAspectRatio() const { return aspectRatio; }

    void Display();

    void CopyBuffer(GPUPixel* buffer);

private:
    int tamX;
    int tamY;

    float aspectRatio;

    bool missingHeader = true;

    PixelToaster::Display window;

    PixelToaster::TrueColorPixel* pixels;
};

#endif
