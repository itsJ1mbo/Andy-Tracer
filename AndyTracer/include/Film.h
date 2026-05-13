#ifndef _FILM_H
#define _FILM_H

#include "Color.h"
#include "PixelToaster/include/PixelToaster.h"
#include <iostream>

class Film 
{
public:
    Film(int x, int y);

    void AddPixel(Color color);

    int GetTamX() const { return tamX; }
    int GetTamY() const { return tamY; }
    float GetAspectRatio() const { return aspectRatio; }

    void Display();

private:
    int tamX;
    int tamY;

    float aspectRatio;

    bool missingHeader = true;

    PixelToaster::Display window;

    std::vector<PixelToaster::TrueColorPixel> pixels;
};

#endif
