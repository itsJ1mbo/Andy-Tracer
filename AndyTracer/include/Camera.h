#ifndef CAMERA_H
#define CAMERA_H

#include "glm/vec3.hpp"
#include "Film.h"
#include "Ray.h"

class Camera {
public:
    Camera(
        glm::vec3 position,
        glm::vec3 look,
        glm::vec3 up,
        const Film &film,
        const float fov_degrees_vertical, 
        float blurA,
        float focalDist
    );

    Ray GetRay(int x, int y) const;

    inline glm::vec3 GetPos() const { return position; }

    glm::vec3 RandomEnCirculo() const;

private:
    glm::vec3 position;
    glm::vec3 delta_x;
    glm::vec3 delta_y;
    glm::vec3 position_top_left;

    glm::vec3 desenfoqueU;
    glm::vec3 desenfoqueV;
};

#endif
