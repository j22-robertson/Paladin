//
// Created by James Robertson on 29/01/2026.
//

#include "RenderApplication.h"

#include "asset/Mesh.h"


RenderApplication::~RenderApplication() {


    m_render_context.reset();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    PALADIN_LOG(INFO, "ENDING APPLICATION")
}

void RenderApplication::run() {
    Setup();
    while (!glfwWindowShouldClose(window)) {
        FrameMark;

        if (!m_render_context->Render())
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
        glfwPollEvents();
    }
}

void RenderApplication::Setup() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    AssetHandle<ModelAsset> sponza = {};

    m_asset_registry = std::make_unique<AssetRegistry>();
    {
        PALADIN_SCOPED_CPU_PROFILE("Loading Sponza",ProfileColors::Blue );
        sponza = m_asset_registry->ImportAsset<ModelAsset>("Sponza.gltf");
    }
    if (m_asset_registry->GetAsset<ModelAsset>(sponza)!=nullptr) {
        PALADIN_LOG(INFO, "sponza valid")
    }
    else {
        PALADIN_LOG(INFO, "sponza invalid")
    }


    {
        PALADIN_SCOPED_CPU_PROFILE("Removing Sponza",ProfileColors::Blue );
        m_asset_registry->Remove<ModelAsset>(sponza);
    }


    if (m_asset_registry->GetAsset<ModelAsset>(sponza)!=nullptr) {
        PALADIN_LOG(INFO, "sponza valid")
    }
    else {
        PALADIN_LOG(INFO, "sponza invalid")
    }


    RingBuffer<int> ring_buffer = RingBuffer<int>(3u);

    for (int i = 0; i < ring_buffer.Capacity()+2; i++) {
        ring_buffer.PushBack(i);
    }
    for (int i = 0; i < ring_buffer.Capacity(); i++) {
        if (auto front = ring_buffer.GetFront(); front.has_value()) {
            PALADIN_LOG(INFO, "Front:"+std::to_string(front.value()))

        }
        if (auto back= ring_buffer.GetBack(); back.has_value()) {
            PALADIN_LOG(INFO, "Back:"+std::to_string(back.value()))
        }
    }

    ring_buffer.PopFront();
    ring_buffer.PopFront();
    for (auto value : ring_buffer.GetForPrint()) {
        PALADIN_LOG(INFO, "Value:"+std::to_string(value))
    }
    PALADIN_LOG(INFO, "RingBuffer count:" + std::to_string(ring_buffer.Count()))


    glfwInit();
    window = glfwCreateWindow(window_width, window_height, "Paladin-Triangle", nullptr, nullptr);


    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwSetWindowUserPointer(window,this);


    glfwSetFramebufferSizeCallback(window,[](GLFWwindow* window, int width, int height) {
        auto application = static_cast<RenderApplication*>(glfwGetWindowUserPointer(window));
        auto clamped_width = std::clamp(width, 100,1920);
        auto clamped_height = std::clamp(height, 100, 1080);


        application->window_width = clamped_width;
        application->window_height = clamped_height;
        application->m_render_context->OnResize(clamped_width,clamped_height);
    });

    if (window == nullptr) {
        glfwTerminate();
        PALADIN_LOG(ERR, "Failed to create GLFW window")
    }
    auto hwnd = glfwGetWin32Window(window);
    ImGui_ImplGlfw_InitForOther(window,true);
    m_render_context = std::make_shared<D3D12Context>(hwnd, window_width,window_height);
}

bool RenderApplication::Update(float delta_time) {
    return true;
}

void RenderApplication::Render(float delta_time) {

}
