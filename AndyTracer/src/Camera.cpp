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
    const float fovDegreesVertical,
    float blurAngle,
    float focalDist
) : position(position) {
    const float fovRadiansVertical = glm::radians(fovDegreesVertical * 0.5);
    const float halfHeightNormalized = std::tan(fovRadiansVertical);

    const glm::vec3 forwardDisplacement = position - look;
    const float focalLength = glm::length(forwardDisplacement);
    const glm::vec3 forward = forwardDisplacement / focalLength;
    const glm::vec3 right = glm::cross(up, forward);    
    const glm::vec3 v = glm::cross(forward, right);

    const float halfHeightViewport = focalLength * halfHeightNormalized;
    const float halfWidthViewport = halfHeightViewport * film.GetAspectRatio();

    const float heightViewport = halfHeightViewport * 2.0f;
    const float widthViewport = halfWidthViewport * 2.0f;

    const float pixelHeight = heightViewport / float(film.GetTamY());
    const float pixelWidth = widthViewport / float(film.GetTamX());

    deltaX = right * pixelWidth;
    deltaY = -v * pixelHeight;
    positionTopLeft =
        position - focalLength * forward
        + v * halfHeightViewport + deltaX * 0.5f
        - right * halfWidthViewport + deltaY * 0.5f;

    float blurRadius = focalDist * tan(glm::radians(blurAngle) / 2);
    blurU = right * blurRadius;
    blurV = v * blurRadius;
}

Ray Camera::GetRay(int x, int y) const {
    const glm::vec3 sample =
        positionTopLeft + deltaX * (float)x + deltaY * (float)y;

    glm::vec3 p = RandomEnCirculo();
    glm::vec3 origen = position + p.x * blurU + p.y * blurV;

    const glm::vec3 dir = (sample - origen);

    return Ray{origen, glm::normalize(dir)};
}

glm::vec3 Camera::RandomEnCirculo() const
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(0, 1);

    const float theta = distrib(gen) * 2 * glm::pi<float>();

    return glm::vec3(cos(theta), sin(theta), 0);
}
