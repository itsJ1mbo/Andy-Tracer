#include "Camera.h"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtc/constants.hpp"
#include <random>

Camera::Camera(
    glm::vec3 position,
    glm::vec3 look,
    glm::vec3 up,
    const Film &film,
    const float fov_degrees_vertical,
    float blurAngle,
    float focalDist
) : position(position) {
    const float fov_radians_vertical = glm::radians(fov_degrees_vertical * 0.5);
    const float half_height_normalized = std::tan(fov_radians_vertical);

    const glm::vec3 forward_displacement = position - look;
    const float focal_length = glm::length(forward_displacement);
    const glm::vec3 forward = forward_displacement / focal_length;
    const glm::vec3 right = glm::cross(up, forward);    
    const glm::vec3 v = glm::cross(forward, right);

    const float half_height_viewport = focal_length * half_height_normalized;
    const float half_width_viewport = half_height_viewport * film.GetAspectRatio();

    const float height_viewport = half_height_viewport * 2.0;
    const float width_viewport = half_width_viewport * 2.0;

    const float pixel_height = height_viewport / float(film.GetTamY());
    const float pixel_width = width_viewport / float(film.GetTamX());

    delta_x = right * pixel_width;
    delta_y = -v * pixel_height;
    position_top_left =
        position - focal_length * forward
        + v * half_height_viewport + delta_x * 0.5f
        - right * half_width_viewport + delta_y * 0.5f;

    float radioDesenfoque = focalDist * tan(glm::radians(blurAngle) / 2);
    desenfoqueU = right * radioDesenfoque;
    desenfoqueV = v * radioDesenfoque;
}

Ray Camera::GetRay(int x, int y) const {
    const glm::vec3 sample =
        position_top_left + delta_x * (float)x + delta_y * (float)y;

    glm::vec3 p = RandomEnCirculo();
    glm::vec3 origen = position + p.x * desenfoqueU + p.y * desenfoqueV;

    const glm::vec3 dir = (sample - origen);

    return Ray{origen, glm::normalize(dir)};
}

glm::vec3 Camera::RandomEnCirculo() const
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(0, 1);

    float theta = distrib(gen) * 2 * glm::pi<float>();

    return glm::vec3(cos(theta), sin(theta), 0);
}
