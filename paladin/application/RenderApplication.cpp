//
// Created by James Robertson on 29/01/2026.
//

#include "RenderApplication.h"

#include "Scene.h"
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
   // io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

   // Input::keys[1024] = {false};

    AssetHandle<ModelAsset> sponza = {};
    AssetHandle<ModelAsset> sponza_two = {};
    m_asset_registry = std::make_unique<AssetRegistry>();
    {
        PALADIN_SCOPED_CPU_PROFILE("Loading Sponza",ProfileColors::Blue );
        sponza = m_asset_registry->ImportAsset<ModelAsset>("Sponza.gltf");
        sponza_two =  m_asset_registry->ImportAsset<ModelAsset>("Sponza.gltf");
    }
    if (m_asset_registry->GetAsset<ModelAsset>(sponza)!=nullptr) {
        PALADIN_LOG(INFO, "sponza valid")
    }


    std::vector<Paladin::Vertex> all_vertices = std::vector<Paladin::Vertex>();
    std::vector<std::uint32_t> all_indices = std::vector<std::uint32_t>();

    std::vector<MeshRange> mesh_ranges = std::vector<MeshRange>();

    EntityTest testing = {.transform = Transform(), .model_handle = sponza};


    std::vector<std::pair<std::uint32_t,TextureRange>> mesh_to_albedo;

#include <cstddef>

    PALADIN_LOG(INFO, "Offset Pos: " + std::to_string(offsetof(Paladin::Vertex, x)));
    PALADIN_LOG(INFO, "Offset Normal: " + std::to_string(offsetof(Paladin::Vertex, nx)));
    PALADIN_LOG(INFO, "Offset Tangent: " + std::to_string(offsetof(Paladin::Vertex, tx))); // Check this!
    PALADIN_LOG(INFO, "Offset Bitangent: " + std::to_string(offsetof(Paladin::Vertex, btx))); // Check this!
    PALADIN_LOG(INFO, "Offset Color: " + std::to_string(offsetof(Paladin::Vertex, r)));
    PALADIN_LOG(INFO, "Offset UV: " + std::to_string(offsetof(Paladin::Vertex, u)));

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
        application->m_camera.aspect_ratio = (float)clamped_width/(float)clamped_height;
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

    std::unique_ptr<GPUModel> gpu_model = std::make_unique<GPUModel>();
    //std::set<OpaqueAssetHandle> material_set;



    auto model = m_asset_registry->GetAsset<ModelAsset>(testing.model_handle);
    gpu_model->mesh_to_material = model->mesh_to_material;
    for (int i = 0; i < model->meshes.size(); i++)
    {
        auto mesh = m_asset_registry->GetAsset<MeshAsset>(model->meshes[i]);
        gpu_model->mesh_handles.push_back(m_render_context->UploadMesh(*mesh, model->meshes[i]));

        auto material_handle = model->materials[model->mesh_to_material[i]];
        auto material = m_asset_registry->GetAsset<MaterialAsset>(material_handle);
        auto albedo_handle = material->GetTexture(Albedo);
        auto roughness_handle = material->GetTexture(Roughness);
        auto metallic_handle = material->GetTexture(Metallic);
        auto normal_handle = material->GetTexture(Normal);

        std::unique_ptr<GPUMaterial> gpu_material = std::make_unique<GPUMaterial>();

        auto albedo_texture = m_asset_registry->GetAsset<Texture2DAsset>(albedo_handle);
        if (albedo_texture != nullptr) {
            gpu_material->albedo = m_render_context->UploadTexture2D(*albedo_texture, albedo_handle);
        }
        auto roughness_texture = m_asset_registry->GetAsset<Texture2DAsset>(roughness_handle);
        if (roughness_texture != nullptr) {
            gpu_material->roughness= m_render_context->UploadTexture2D(*roughness_texture, roughness_handle);

        }
        auto metallic_texture =  m_asset_registry->GetAsset<Texture2DAsset>(metallic_handle);
        if (metallic_texture != nullptr) {
            gpu_material->metallic =m_render_context->UploadTexture2D(*metallic_texture, metallic_handle);
        }
        auto normal_texture = m_asset_registry->GetAsset<Texture2DAsset>(normal_handle);
        if (normal_texture != nullptr) {
            gpu_material->normal= m_render_context->UploadTexture2D(*normal_texture, normal_handle);
            //PALADIN_LOG(INFO, "normal null")
        }
        gpu_model->material_handles.push_back( m_render_context->AddMaterial( std::move(gpu_material), material_handle));
    }
    m_render_context->AddModel(std::move(gpu_model), sponza);
    std::unique_ptr<GPUModel> gpu_model_two = std::make_unique<GPUModel>();
    auto m_two = m_asset_registry->GetAsset<ModelAsset>(sponza_two);
    gpu_model_two->mesh_to_material = m_two->mesh_to_material;
    for (int i = 0; i < m_two->meshes.size(); i++)
    {
        auto mesh = m_asset_registry->GetAsset<MeshAsset>(m_two->meshes[i]);
        gpu_model_two->mesh_handles.push_back(m_render_context->UploadMesh(*mesh, m_two->meshes[i]));

        auto material_handle = m_two->materials[m_two->mesh_to_material[i]];
        auto material = m_asset_registry->GetAsset<MaterialAsset>(material_handle);
        auto albedo_handle = material->GetTexture(Albedo);
        auto roughness_handle = material->GetTexture(Roughness);
        auto metallic_handle = material->GetTexture(Metallic);
        auto normal_handle = material->GetTexture(Normal);

        std::unique_ptr<GPUMaterial> gpu_material = std::make_unique<GPUMaterial>();

        auto albedo_texture = m_asset_registry->GetAsset<Texture2DAsset>(albedo_handle);
        if (albedo_texture != nullptr) {
            gpu_material->albedo = m_render_context->UploadTexture2D(*albedo_texture, albedo_handle);
        }
        auto roughness_texture = m_asset_registry->GetAsset<Texture2DAsset>(roughness_handle);
        if (roughness_texture != nullptr) {
            gpu_material->roughness= m_render_context->UploadTexture2D(*roughness_texture, roughness_handle);

        }
        auto metallic_texture =  m_asset_registry->GetAsset<Texture2DAsset>(metallic_handle);
        if (metallic_texture != nullptr) {
            gpu_material->metallic =m_render_context->UploadTexture2D(*metallic_texture, metallic_handle);
        }
        auto normal_texture = m_asset_registry->GetAsset<Texture2DAsset>(normal_handle);
        if (normal_texture != nullptr) {
            gpu_material->normal= m_render_context->UploadTexture2D(*normal_texture, normal_handle);
            //PALADIN_LOG(INFO, "normal null")
        }
        gpu_model_two->material_handles.push_back( m_render_context->AddMaterial( std::move(gpu_material), material_handle));
    }
    m_render_context->AddModel(std::move(gpu_model_two), sponza_two);
    //m_render_context->UploadModel(all_vertices, all_indices,mesh_ranges);
    m_render_context->CreatePersistantAllocation(m_camera.GetUniformMut());
    m_render_context->CreateInstanceBuffer();


    auto sponza_model = m_asset_registry->GetAsset<ModelAsset>(sponza);
    for (int x = 1; x < 3; x++) {
        for (int z =1; z < 3; z++) {
            auto transform = Transform{};
            transform.SetPosition({x*5000,0,z*5000});
            transform.SetScale({1,1,1});
            //instancing_test_data.push_back(transform.GetData());
            frame_data.insert(sponza, transform);
            transforms.push_back(transform);
            for (int i = 0; i < sponza_model->meshes.size(); i++)
            {
                const auto mesh = m_asset_registry->GetAsset<MeshAsset>(sponza_model->meshes[i]);
                auto& aabb = mesh->bounding_box;
                auto aabb_center = aabb.center;
                auto position = transform.GetPosition()+aabb_center;

                auto aabb_real_transform = Transform{};
                aabb_real_transform.SetPosition(transform.GetPosition());
                aabb_real_transform.SetScale(glm::vec3(1.0));
                AABB_real_transforms.push_back(aabb_real_transform);

                auto aabb_transform = Transform{};
                aabb_transform.SetPosition(position);
                aabb_transform.SetScale(glm::abs(aabb.maximum-aabb.minimum));

                AABB_transforms.push_back(aabb_transform);
                frame_data.debug_aabb_transforms.push_back(aabb_transform.GetData());
                AABBs.push_back(aabb);
            }
        }
    }

    for (int x = 1; x < 3; x++) {
        for (int z = 1; z <3; z++) {
            auto transform = Transform{};
            transform.SetPosition({-x*5000,0,-z*5000});
            transform.SetScale({1,1,1});
            frame_data.insert(sponza_two, transform);
            transforms.push_back(transform);
        }
    }

    //m_render_context->UploadFrameData(m_camera);
}

bool RenderApplication::Update(float delta_time) {
    m_camera.direction = glm::vec3(0.0);


   // auto& tf = transforms[99+4];
   // tf.Rotate(Axis::X_AXIS,rot_test+=0.00001f * delta_time);
    
    //frame_data.batches[1].transforms[3] = tf.GetData();
/*
    frame_data.debug_aabb_transforms.clear();
    for (int i = 0; i < AABBs.size(); i++) {
       // if (m_camera.IsOnFrustrum(AABBs[i],AABB_real_transforms[i])) {
            frame_data.debug_aabb_transforms.push_back(AABB_transforms[i].GetData());
       // }
    }*/

    m_render_context->UpdateRenderFrameData(frame_data);

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
