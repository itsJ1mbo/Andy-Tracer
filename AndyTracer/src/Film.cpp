#include "Film.h"

void Film::AddPixel(Color color) {
    if (missingHeader) {
        _out << "P3\n" << _tamX << ' ' << _tamY << "\n255\n";
        missingHeader = false;
    }

    int ir = (int)(255.99 * color.x);
    int ig = (int)(255.99 * color.y);
    int ib = (int)(255.99 * color.z);

    _out << ir << ' ' << ig << ' ' << ib << '\n';
}
