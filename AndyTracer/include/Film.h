#ifndef _FILM_H
#define _FILM_H

#include "Color.h"
#include <iostream>

class Film 
{
public:
    Film(int x, int y, std::ostream& output) : tamX(x), tamY(y), out(output), aspectRatio(1.0f*tamX / tamY) {}

    Film(int x, int y) : Film(x, y, std::cout) {}

    void AddPixel(Color color);

    int GetTamX() const { return tamX; }
    int GetTamY() const { return tamY; }
    float GetAspectRatio() const { return aspectRatio; }

private:
    int tamX;
    int tamY;

    std::ostream& out;

    float aspectRatio;

    bool missingHeader = true;
};

#endif
