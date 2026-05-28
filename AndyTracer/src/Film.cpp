#include "Film.h"

Film::Film(int x, int y) : tamX(x), tamY(y), aspectRatio(1.0f * tamX / tamY), window(PixelToaster::Display("AndyTracer", x, y, PixelToaster::Output::Windowed))
{
    pixels = new PixelToaster::TrueColorPixel[tamX*tamY];
}

void Film::AddPixel(Color color, int x, int y) {
    int ir = (int)(255.99 * color.x);
    int ig = (int)(255.99 * color.y);
    int ib = (int)(255.99 * color.z);

    int index = y * tamX + x;
    pixels[index] = (PixelToaster::TrueColorPixel(ir, ig, ib, 255));
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
