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
        Update(0.0);
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

   // Input::keys[1024] = {false};

    AssetHandle<ModelAsset> sponza = {};

    m_asset_registry = std::make_unique<AssetRegistry>();
    {
        PALADIN_SCOPED_CPU_PROFILE("Loading Sponza",ProfileColors::Blue );
        sponza = m_asset_registry->ImportAsset<ModelAsset>("Sponza.gltf");
    }
    if (m_asset_registry->GetAsset<ModelAsset>(sponza)!=nullptr) {
        PALADIN_LOG(INFO, "sponza valid")
    }


    std::vector<Paladin::Vertex> all_vertices;
    std::vector<std::uint32_t> all_indices;

    std::vector<MeshRange> mesh_ranges;

    EntityTest testing = {.transform = Transform(), .model_handle = sponza};

    for (auto mesh_handle : m_asset_registry->GetAsset<ModelAsset>(testing.model_handle)->meshes) {
        auto mesh = m_asset_registry->GetAsset<MeshAsset>(mesh_handle);

        auto v_start = all_vertices.size();
        auto i_start = all_indices.size();

        all_indices.insert(all_indices.end(), mesh->indices.begin(), mesh->indices.end());
        all_vertices.insert(all_vertices.end(), mesh->vertices.begin(), mesh->vertices.end());


        auto i_end = all_indices.size()-1;
        auto v_end = all_vertices.size()-1;

        mesh_ranges.push_back(MeshRange{
        .start_v = v_start,
        .end_v = v_end,
        .start_i = i_start,
        .end_i = i_end,});
    }


    glfwInit();
    window = glfwCreateWindow(window_width, window_height, "Paladin-Triangle", nullptr, nullptr);


    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwSetWindowUserPointer(window,this);


    glfwSetKeyCallback(window, Input::InputCallback);

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

    m_render_context->UploadModel(all_vertices, all_indices, mesh_ranges);



}

bool RenderApplication::Update(float delta_time) {
    if (Input::keys[GLFW_KEY_W]) {
        PALADIN_LOG(INFO, "Key W press");
    }

    return true;
}

void RenderApplication::Render(float delta_time) {

}
