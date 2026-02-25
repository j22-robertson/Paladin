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
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        glfwPollEvents();
    }
}

void RenderApplication::Setup() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags  |= ImGuiConfigFlags_ViewportsEnable;
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


    std::vector<unsigned char> all_albedo_textures = std::vector<unsigned char>();
    std::vector<TextureRange> texture_ranges = std::vector<TextureRange>();

    std::map<AssetHandle<MaterialAsset>,std::vector<AssetHandle<Texture2DAsset>>> material_handle_to_texture_handle;
    std::map<AssetHandle<Texture2DAsset>, std::uint32_t> texture_handle_to_range;

    for (auto material_handle : m_asset_registry->GetAsset<ModelAsset>(testing.model_handle)->materials) {
        auto material = m_asset_registry->GetAsset<MaterialAsset>(material_handle);

        auto albedo_handle = material->GetTexture(Albedo);
        auto albedo_texture = m_asset_registry->GetAsset<Texture2DAsset>(albedo_handle);
        TextureRange texture_range = {};
        texture_range.channel = albedo_texture->channels;
        texture_range.height = albedo_texture->height;
        texture_range.width = albedo_texture->width;
        texture_range.start = all_albedo_textures.size();
        texture_range.size = albedo_texture->pixels.size();
        //texture_handle_to_range.insert(std::make_pair(albedo_handle,texture_ranges.size()));
        texture_ranges.push_back(texture_range);

        all_albedo_textures.insert(all_albedo_textures.end(),albedo_texture->pixels.begin(),albedo_texture->pixels.end());
    }

    std::vector<std::pair<std::uint32_t,TextureRange>> mesh_to_albedo;

    for (auto mesh_handle : m_asset_registry->GetAsset<ModelAsset>(testing.model_handle)->meshes) {
        auto mesh = m_asset_registry->GetAsset<MeshAsset>(mesh_handle);
        auto material = m_asset_registry->GetAsset<MaterialAsset>(mesh->material);

        auto albedo_handle = material->GetTexture(Albedo);

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

    m_render_context->UploadModel(all_vertices, all_indices,all_albedo_textures,mesh_ranges,texture_ranges);
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
