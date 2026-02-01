//
// Created by James Robertson on 29/01/2026.
//

#include "RenderApplication.h"

RenderApplication::~RenderApplication() {

    if (m_render_context) {
        delete m_render_context;
        m_render_context = nullptr;
    }
    glfwTerminate();
}

void RenderApplication::run() {
    Setup();

    while (!glfwWindowShouldClose(window)) {
        if (!m_render_context->Render())
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
        glfwPollEvents();
    }
}

void RenderApplication::Setup() {
    std::cout << "Hello World from Paladin"<< std::endl;
    glfwInit();
    window = glfwCreateWindow(window_width, window_height, "Paladin-Triangle", nullptr, nullptr);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (window == nullptr) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
    }
    auto hwnd = glfwGetWin32Window(window);

    m_render_context = new D3D12Context(hwnd, window_width, window_height);
}

bool RenderApplication::Update(float delta_time) {
    return true;
}

void RenderApplication::Render(float delta_time) {

}
