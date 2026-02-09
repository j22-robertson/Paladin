//
// Created by James Robertson on 05/02/2026.
//

#ifndef PALADIN_TRANSFORM_H
#define PALADIN_TRANSFORM_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Axis {
    constexpr glm::vec3 X_AXIS{1.0f, 0.0f, 0.0f};
    constexpr glm::vec3 Y_AXIS{0.0f, 1.0f, 0.0f};
    constexpr glm::vec3 Z_AXIS{0.0f, 0.0f, 1.0f};
    constexpr glm::vec3 EVERY_AXIS{1.0,1.0,1.0};
}

//TODO: Local and Global for parenting?
class Transform {
    Transform() = default;

    [[nodiscard]] bool HasUpdated() const {
        return has_updated;
    }

    [[nodiscard]] const glm::vec3& GetPosition() const {
        return position;
    }

    void SetScale(const glm::vec3& new_scale) {
        has_updated = true;
        scale = new_scale;
    }


    void SetPosition(const glm::vec3& new_position) {
        has_updated = true;
        position = new_position;
    }

    //Use Axis constexpr variables
    void Rotate(const glm::vec3& axis,float angle_degrees) {
        has_updated = true;
        rotation = glm::rotate(rotation, angle_degrees, axis);
    }

    const glm::mat4& GetMatrix() {
        if (!has_updated) return matrix;
        glm::mat4 T = glm::translate(glm::mat4(1.0f),position);
        glm::mat4 R= glm::toMat4(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0), scale);

        has_updated = false;

        matrix = T*R*S;
        return matrix;
    }

private:
    bool has_updated = false;

    glm::vec3 position = glm::vec3(0);
    glm::vec3 scale = glm::vec3(0);
    glm::quat rotation = glm::vec3(0);

    glm::mat4 matrix = glm::mat4(1.0);

};


#endif //PALADIN_TRANSFORM_H