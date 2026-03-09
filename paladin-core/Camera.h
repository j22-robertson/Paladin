//
// Created by James Robertson on 05/02/2026.
//

#ifndef PALADIN_CAMERA_H
#define PALADIN_CAMERA_H
#include "Transform.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "utility/BoundingBox.h"

struct CameraUniform {
    glm::mat4 view = glm::mat4(1.0);
    glm::mat4 projection = glm::mat4(1.0);
};



struct Frustum {
    Plane top_face;
    Plane bottom_face;

    Plane right_face;
    Plane left_face;

    Plane near_face;
    Plane far_face;
};

class Camera {
public:
   Camera()=default;


    void Update(const float delta) {
        if (direction == glm::vec3(0.0f,0.0f,0.0f)) {
            return;
        }
        up_to_date=false;

        auto mapped_direction = (direction.x*right+direction.y*up+ direction.z*forward);
        velocity = mapped_direction*speed;
        position +=velocity * delta;
    }

    const CameraUniform& GetUniform() {
        if (!up_to_date) {
            uniform.view = ViewMatrix();
            uniform.projection = GetProjection();
        }
        up_to_date = true;
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
        yaw += offset_x;
        pitch+= offset_y;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        last_x = current_mouse_x;
        last_y = current_mouse_y;

        UpdateCameraVectors();
        ViewFrustum();
    }

    //https://iquilezles.org/articles/frustum/
// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    Frustum ViewFrustum() {
        const auto& right_ =right;
        const auto& up_ =up;
        const auto& forward_ = forward;

        auto m = uniform.projection*uniform.view;
        glm::vec4 r0(m[0][0], m[1][0], m[2][0], m[3][0]);
        glm::vec4 r1(m[0][1], m[1][1], m[2][1], m[3][1]);
        glm::vec4 r2(m[0][2], m[1][2], m[2][2], m[3][2]);
        glm::vec4 r3(m[0][3], m[1][3], m[2][3], m[3][3]);
        frustum.left_face = NormalizePlane(r3+r0);
        frustum.right_face= NormalizePlane(r3-r0);

        frustum.bottom_face = NormalizePlane(r3+r1);
        frustum.top_face = NormalizePlane(r3-r1);

        frustum.near_face = NormalizePlane(r2);
        frustum.far_face = NormalizePlane(r3-r2);
        return frustum;
    }
    // https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    bool IsOnFrustrum(const AABB&  aabb,Transform& transform) {
        const auto& transform_data = transform.GetData();
        const auto& model_matrix = transform_data.model;
        const glm::vec3 world_location = model_matrix * glm::vec4(aabb.center, 1.0f);

        const glm::vec3 orientation_right = transform.Right() * aabb.extent.x;
        const glm::vec3 orientation_up = transform.Up() * aabb.extent.y;
        const glm::vec3 orientation_forward = transform.Forward() * aabb.extent.z;
/*
        const float new_Ii = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, orientation_right)) +
        std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, orientation_up)) +
        std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, orientation_forward));

        const float new_Ij = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, orientation_right)) +
                std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, orientation_up)) +
                std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, orientation_forward));

        const float new_Ik = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, orientation_right)) +
              std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, orientation_up)) +
              std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, orientation_forward));*/
        float new_x = std::abs(orientation_right.x) + std::abs(orientation_up.x) + std::abs(orientation_forward.x);
        float new_y = std::abs(orientation_right.y) + std::abs(orientation_up.y) + std::abs(orientation_forward.y);
        float new_z = std::abs(orientation_right.z) + std::abs(orientation_up.z) + std::abs(orientation_forward.z);
        const auto world_aabb = AABB(world_location, new_x,new_y, new_z);

        return (world_aabb.IsOnForwardPlane(frustum.left_face) &&
            world_aabb.IsOnForwardPlane(frustum.right_face) &&
            world_aabb.IsOnForwardPlane(frustum.top_face) &&
            world_aabb.IsOnForwardPlane(frustum.bottom_face) &&
            world_aabb.IsOnForwardPlane(frustum.near_face) &&
            world_aabb.IsOnForwardPlane(frustum.far_face));
    }

    void UpdateCameraVectors() {
        forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward.y = sin(glm::radians(pitch));
        forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward = glm::normalize(forward);
        right = glm::normalize(glm::cross(forward,glm::vec3(0.0,1.0,0.0)));
        up = glm::normalize(glm::cross(right,forward));
    }

    glm::vec3 velocity = glm::vec3(0.0);

    bool up_to_date = false;

    float speed = 2000.0f;
    float rotation_speed = 1000.0;

    CameraUniform uniform;

    glm::vec3 direction = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);

    float width = 0;
    float height = 0;
    float aspect_ratio = static_cast<float>(1920) / static_cast<float>(1080);
private:
    glm::vec3 forward = glm::vec3(0.0f);
    glm::vec3 up= glm::vec3(0.0f);;
    glm::vec3 right= glm::vec3(0.0f);;
    float last_x = 0.0;
    float last_y = 0.0;

    float pitch =0.0f;
    float yaw =-90.0f;

    float fov = 90.0f;

    float z_near = 1.0f;
    float z_far = 500000.0f;
    Frustum frustum = {};
    [[nodiscard]] glm::mat4 RotationMatrix() const
    {
        const glm::quat pitch_rotation = glm::angleAxis(glm::radians(pitch), glm::vec3{ 1.f,0.0f,0.0f });
        const glm::quat yaw_rotation= glm::angleAxis(glm::radians(yaw), glm::vec3{ 0.f,1.f,0.f });
        return glm::toMat4(yaw_rotation) * glm::toMat4(pitch_rotation);
    }
    [[nodiscard]] glm::mat4 ViewMatrix() {
        return glm::lookAt(position, position+forward, up);
    }
    [[nodiscard]] glm::mat4 GetProjection() const {
        return glm::perspective(glm::radians(fov), aspect_ratio, z_near, z_far);
    }
};
#endif //PALADIN_CAMERA_H