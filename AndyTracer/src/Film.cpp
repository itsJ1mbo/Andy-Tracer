#include "Film.h"

void Film::AddPixel(Color color) {
    if (missingHeader) {
        out << "P3\n" << tamX << ' ' << tamY << "\n255\n";
        missingHeader = false;
    }

    int ir = (int)(255.99 * color.x);
    int ig = (int)(255.99 * color.y);
    int ib = (int)(255.99 * color.z);

    out << ir << ' ' << ig << ' ' << ib << '\n';
}
