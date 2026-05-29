#include "Film.h"
#include "defs.h"

Film::Film(int x, int y) : tamX(x), tamY(y), aspectRatio(1.0f * tamX / tamY), window(PixelToaster::Display("AndyTracer", x, y, PixelToaster::Output::Windowed))
{
    pixels = new PixelToaster::TrueColorPixel[tamX*tamY];
}

Film::~Film()
{
    delete[] pixels;
    pixels = nullptr;
}

void Film::Display()
{
    window.update(pixels);
}

void Film::CopyBuffer(GPUPixel* buffer)
{
    size_t totalBytes = tamX * tamY * sizeof(GPUPixel);
    memcpy(pixels, buffer, totalBytes);
}
