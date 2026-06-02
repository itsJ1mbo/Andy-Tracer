#pragma once

#include "AABB.h"
#include "Shape.h"
#include "Light.h"

struct Scene : Shape
{
    int shapeCount;
    Shape* shapes;

    int lightCount;
    Light* lights;

    int nodesCount;
    BVHNode* bvhNodes;

    __host__ Scene() : shapeCount(0), shapes(nullptr), lightCount(0), lights(nullptr) {}

    __device__ bool Intersect(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const
    {
        bool hitAnything = false;
        float closestT = tMax;

        for (int i = 0; i < shapeCount; i++)
        {
            ShapeIntersection tempInfo;
            if (shapes[i].Intersect(ray, tMin, closestT, tempInfo))
            {
                hitAnything = true;
                closestT = tempInfo.t; 
                info = tempInfo;
            }
        }

        return hitAnything;
    }

    __device__ bool IntersectBVH(const Ray& ray, float tMin, float tMax, ShapeIntersection& info) const
    {
        bool hitAnything = false;
        float closestT = tMax;

        int stack[64];
        int stackPtr = 0;

        stack[stackPtr++] = 0;

		// Mientras haya nodos por visitar
        while (stackPtr > 0)
        {
            // Se saca un nodo del stack
            int nodeIdx = stack[--stackPtr];
            BVHNode& node = bvhNodes[nodeIdx];

			// Si el rayo intersecta el AABB del nodo
            if (node.bounds.Intersect(ray, tMin, closestT))
            {
				// En caso de ser hoja hay formas con las que intesercetar, sino se añaden los hijos al stack
                if (node.isLeaf)
                {
                    for (int i = 0; i < node.rightChild; ++i)
                    {
                        ShapeIntersection tempInfo;
                        if (shapes[node.firstShape + i].Intersect(ray, tMin, closestT, tempInfo))
                        {
                            // Profundidad
                            hitAnything = true;
                            closestT = tempInfo.t;
                            info = tempInfo;
                        }
                    }
                }
                else
                {
                    stack[stackPtr++] = node.leftChild;
                    stack[stackPtr++] = node.rightChild;
                }
            }
        }

        return hitAnything;
    }
};