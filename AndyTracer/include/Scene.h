#pragma once
#include "Shape.h"

struct Scene : Shape
{
    int count;
    Shape* shapes;

    __host__ Scene() : count(0), shapes(nullptr) {}
};