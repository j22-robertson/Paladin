//
// Created by James Robertson on 05/02/2026.
//

#ifndef PALADIN_CAMERA_H
#define PALADIN_CAMERA_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

struct CameraUniform {
    glm::mat4 view = glm::mat4(1.0);
    glm::mat4 projection = glm::mat4(1.0);
};


class Camera {
public:
   Camera()=default;


    void Update(const float delta) {
        if (direction == glm::vec3(0.0f,0.0f,0.0f)) {
            return;
        }
        velocity = direction*speed;
        up_to_date=false;
        const auto rot = RotationMatrix();
        position +=  glm::vec3( rot* glm::vec4(velocity, 0.0f));
    }

    const CameraUniform& GetUniform() {
        if (!up_to_date) {
            uniform.view = ViewMatrix();
            uniform.projection = GetProjection();
        }
        return uniform;
    }

    CameraUniform GetUniformMut()
    {
        if (!up_to_date) {
            uniform.view = ViewMatrix();
            uniform.projection = GetProjection();
        }
        up_to_date = true;
        return uniform;
    }

    // Use X and Y mouse coordinates to rotate camera
    void LookAt(const float current_mouse_x, const float current_mouse_y) {
        if (current_mouse_x == last_x && current_mouse_y == last_y) {
            return;
        }
        up_to_date = false;
        float offset_x = current_mouse_x - last_x;
        float offset_y = last_y - current_mouse_y;

        //TODO: maybe use DT?
        yaw += offset_x/rotation_speed;
        pitch -= offset_y/rotation_speed;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        last_x = current_mouse_x;
        last_y = current_mouse_y;
    }

    glm::vec3 velocity = glm::vec3(0.0);


    bool up_to_date = false;

    float speed = 20.0f;
    float rotation_speed = 100.0;

    CameraUniform uniform;
    glm::vec3 direction = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);

    float width = 0;
    float height = 0;

    float last_x = 0.0;
    float last_y = 0.0;

    float pitch =0.0f;
    float yaw =-90.0f;

    float fov = {90.0f};
    float aspect_ratio = static_cast<float>(800) / static_cast<float>(600);
    float near_plane = 1.0f;
    float far_plane = 500000.0f;

private:

    [[nodiscard]] glm::mat4 RotationMatrix() const
    {

        const glm::quat pitch_rotation = glm::angleAxis(pitch, glm::vec3{ 1.f,0.0f,0.0f });

        const glm::quat yaw_rotation= glm::angleAxis(yaw, glm::vec3{ 0.f,1.f,0.f });

        return glm::toMat4(yaw_rotation) * glm::toMat4(pitch_rotation);
    }

    [[nodiscard]] glm::mat4 ViewMatrix() const {

        const glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.0f), position);
        const glm::mat4 cameraRotation = RotationMatrix();
        return glm::inverse(cameraTranslation * cameraRotation);
    }

    [[nodiscard]] glm::mat4 GetProjection() const {
        return glm::perspectiveLH_ZO(glm::radians(fov), aspect_ratio, near_plane, far_plane);
    }
};
#endif //PALADIN_CAMERA_H