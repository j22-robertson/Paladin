//
// Created by James Robertson on 29/01/2026.
//

#include "RenderApplication.h"

RenderApplication::~RenderApplication() {

    glfwTerminate();
}

void RenderApplication::run() {


    Setup();

    //glfwMakeContextCurrent(window);


}

void RenderApplication::Setup() {
    std::cout << "Hello World from Paladin"<< std::endl;
    glfwInit();




    window = glfwCreateWindow(window_width, window_height, "PaladinRenderApp", nullptr, nullptr);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (window == nullptr) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
    }
    auto hwnd = glfwGetWin32Window(window);

    m_render_context = std::make_unique<D3D12Context>(hwnd, window_width, window_height);
    while (!glfwWindowShouldClose(window)) {
        if (!m_render_context->Render())
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
        glfwPollEvents();
    }
}

bool RenderApplication::Update(float delta_time) {
    return true;
}

void RenderApplication::Render(float delta_time) {

}
