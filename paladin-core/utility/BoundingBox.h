//
// Created by James Robertson on 09/03/2026.
//

#ifndef PALADIN_BOUNDINGBOX_H
#define PALADIN_BOUNDINGBOX_H
#include "glm/glm.hpp"
consteval  glm::vec3 MAX_VEC3() {
    return {(std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)()};
}

consteval  glm::vec3 MIN_VEC3() {
    return {(std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)(), (std::numeric_limits<float>::min)()};
}
struct Plane {
    glm::vec3 normal = {0.0f,1.0f,0.0f};
    float distance = 0.0f;
    Plane() = default;
    Plane(const glm::vec3& n, const float d) : normal(n), distance(d){};
    float SDFToPlane(const glm::vec3& point) const
    {
        return glm::dot(normal, point)+distance;
    }
};

inline Plane NormalizePlane(glm::vec4 p) {
    float mag = glm::length(glm::vec3(p));
    return Plane(glm::vec3(p) / mag, p.w / mag);
}
struct AABB
{
    AABB() = default;

    AABB(const glm::vec3 _center, const float extent_x, const float extent_y, const float extent_z) {
        center = _center;
        extent.x = extent_x;
        extent.y = extent_y;
        extent.z = extent_z;

        minimum.x = _center.x - extent_x;
        minimum.y = _center.y - extent_y;
        minimum.z = _center.z - extent_z;

        maximum.x = _center.x + extent_x;
        maximum.y = _center.y + extent_y;
        maximum.z = _center.z + extent_z;
    }

    AABB(glm::vec3 _min, glm::vec3 _max) : minimum(_min), maximum(_max) {
        center = (maximum+minimum)*0.5f;
        extent = {maximum.x-center.x,maximum.y-center.y,maximum.z-center.z};
    }
    bool IsOnForwardPlane(const Plane& plane) const {
        const float r = extent.x * std::abs(plane.normal.x) +
          extent.y * std::abs(plane.normal.y) + extent.z * std::abs(plane.normal.z);
        return plane.SDFToPlane(center)>=-r;
    }
    glm::vec3 minimum = MAX_VEC3();
    glm::vec3 maximum = MIN_VEC3();

    glm::vec3 center = glm::vec3(0.f);
    glm::vec3 extent = glm::vec3(0.f);
};
#endif //PALADIN_BOUNDINGBOX_H