//
// Created by James Robertson on 05/02/2026.
//

#ifndef PALADIN_CAMERA_H
#define PALADIN_CAMERA_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

struct CameraUniform {
    glm::mat4 view;
    glm::mat4 projection;
};


class Camera {
public:
   Camera()=default;


    void update(const float delta) {
        if (direction == glm::vec3(0.0f,0.0f,0.0f)) {
            return;
        }
        up_to_date=false;

        position += direction *speed* delta ;
    }

    const CameraUniform& GetUniform() {
        if (!up_to_date) {
            uniform.view = ViewMatrix();
            uniform.projection = GetProjection();
        }
        return uniform;
    }

    // Use X and Y mouse coordinates to rotate camera
    void LookAt(float current_mouse_x, float current_mouse_y) {
        if (current_mouse_x == last_x && current_mouse_y == last_y) {
            return;
        }
        bool up_to_date = false;
        float offset_x = current_mouse_x - last_x;
        float offset_y = last_y - current_mouse_y;

        //TODO: maybe use DT?
        yaw += offset_x/rotation_speed;
        pitch -= offset_y/rotation_speed;

        last_x = current_mouse_x;
        last_y = current_mouse_y;
    }

private:
    bool up_to_date = true;

    float speed = 100.0;
    float rotation_speed = 100.0;

    CameraUniform uniform;
    glm::vec3 direction = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);

    float last_x = 0.0;
    float last_y = 0.0;

    float pitch {0.0f};
    float yaw {0.f};

    float fov = {90.0f};
    float aspect_ratio = 0.0;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;



    glm::mat4 RotationMatrix() const
    {

        const glm::quat pitch_rotation = glm::angleAxis(pitch, glm::vec3{ 1.f,0.0f,0.0f });

        const glm::quat yaw_rotation= glm::angleAxis(yaw, glm::vec3{ 0.f,-1.f,0.f });

        return glm::toMat4(yaw_rotation) * glm::toMat4(pitch_rotation);
    }

    glm::mat4 ViewMatrix() const {
        const glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.0f), position);
        const glm::mat4 cameraRotation = RotationMatrix();
        return glm::inverse(cameraTranslation * cameraRotation);

    }

    glm::mat4 GetProjection() const {
        return glm::perspective(fov, aspect_ratio, near_plane, far_plane);
    }
};
#endif //PALADIN_CAMERA_H