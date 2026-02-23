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
        float current_time = static_cast<float>(glfwGetTime());
        float delta_time = current_time - m_last_frame_time;
        m_elapsed_time += delta_time;
        m_last_frame_time = current_time;
        Update(delta_time);
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
   // io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
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


    std::vector<Paladin::Vertex> all_vertices = std::vector<Paladin::Vertex>();
    std::vector<std::uint32_t> all_indices = std::vector<std::uint32_t>();

    std::vector<MeshRange> mesh_ranges = std::vector<MeshRange>();

    EntityTest testing = {.transform = Transform(), .model_handle = sponza};

    for (auto mesh_handle : m_asset_registry->GetAsset<ModelAsset>(testing.model_handle)->meshes) {
        auto mesh = m_asset_registry->GetAsset<MeshAsset>(mesh_handle);

        auto v_start = (std::uint32_t)all_vertices.size();
        auto i_start = (std::uint32_t)all_indices.size();

        if (v_start < 200) {
            PALADIN_LOG(INFO, "This should only happen once.")
        }
        all_indices.insert(all_indices.end(), mesh->indices.begin(), mesh->indices.end());
        all_vertices.insert(all_vertices.end(), mesh->vertices.begin(), mesh->vertices.end());


        mesh_ranges.push_back(MeshRange{
            .vertex_start_location = v_start,
            .vertex_count = static_cast<std::uint32_t>(mesh->vertices.size()),
            .index_start_location = i_start,
            .index_count = static_cast<std::uint32_t>(mesh->indices.size())});
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


        application->m_camera.width = clamped_width;
        application->m_camera.height = clamped_height;
        application->m_camera.aspect_ratio = (float)width/(float)height;
        application->window_width = clamped_width;
        application->window_height = clamped_height;
        application->m_render_context->OnResize(clamped_width,clamped_height);
    });


    if (window == nullptr) {
        glfwTerminate();
        PALADIN_LOG(ERR, "Failed to create GLFW window")
    }
    auto hwnd = glfwGetWin32Window(window);
    m_render_context = std::make_shared<D3D12Context>(hwnd, window_width,window_height);
    ImGui_ImplGlfw_InitForOther(window,true);



    m_render_context->UploadModel(all_vertices, all_indices, mesh_ranges,m_camera);
    m_render_context->CreatePersistantAllocation(m_camera.GetUniformMut());
    //m_render_context->UploadFrameData(m_camera);
}

bool RenderApplication::Update(float delta_time) {
    m_camera.direction = glm::vec3(0.0);



    double mouse_x;
    double mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    m_camera.LookAt(mouse_x,mouse_y);



    if (Input::keys[GLFW_KEY_W]) {
        m_camera.direction.z =1;
    }
    if (Input::keys[GLFW_KEY_S]) {
        m_camera.direction.z =-1;
    }
    if (Input::keys[GLFW_KEY_D]) {
        m_camera.direction.x =1;
    }
    if (Input::keys[GLFW_KEY_A]) {
        m_camera.direction.x=-1;
    }
    if (Input::keys[GLFW_KEY_R]) {
        m_camera.direction.y=1;
    }
    if (Input::keys[GLFW_KEY_T]) {
        m_camera.direction.y=-1;
    }
    m_camera.Update(delta_time);

    m_render_context->UpdatePersistantAllocation(m_camera.GetUniformMut());

    //PALADIN_LOG(INFO, "Mouse X:" + std::to_string(mouse_x) + " Mouse Y:" + std::to_string(mouse_y));

    return true;
}

void RenderApplication::Render(float delta_time) {

}
