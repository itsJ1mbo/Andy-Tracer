#include "Film.h"

Film::Film(int x, int y) : tamX(x), tamY(y), aspectRatio(1.0f * tamX / tamY), window(PixelToaster::Display("AndyTracer", x, y, PixelToaster::Output::Windowed))
{
    
}

void Film::AddPixel(Color color) {
    int ir = (int)(255.99 * color.x);
    int ig = (int)(255.99 * color.y);
    int ib = (int)(255.99 * color.z);

    pixels.push_back(PixelToaster::TrueColorPixel(ir, ig, ib, 255));
}

void Film::Display()
{
    window.update(pixels);
    pixels.clear();
}
