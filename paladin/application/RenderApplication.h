//
// Created by James Robertson on 29/01/2026.
//

#ifndef PALADIN_RENDERAPPLICATION_H
#define PALADIN_RENDERAPPLICATION_H
#include "IApplication.h"
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#include <iostream>
#include <memory>

#include "Camera.h"
#include "Transform.h"

#include "profiling/Profiling.h"
#define TRACY_IMPORTS
#include "../rendering/D3D12Context.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "Logger.h"
//TODO: Create a window class
class RenderApplication final : IApplication {
public:
    ~RenderApplication() override;
    void run() override;
    void Setup() override;
    bool Update(float delta_time) override;
    void Render(float delta_time) override;
private:

    bool close_requested = false;

    D3D12Context* m_render_context = nullptr;
    std::uint32_t window_width = 800;
    std::uint32_t window_height = 600;
    GLFWwindow* window = nullptr;

    Camera m_camera{};

    //D3D12Context* m_render_context = nullptr;
};
#endif //PALADIN_RENDERAPPLICATION_H
