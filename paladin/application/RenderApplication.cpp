//
// Created by James Robertson on 29/01/2026.
//

#include "RenderApplication.h"

#include <ranges>
#include <cstddef>
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
        if (!m_render_context->WaitForPreviousFrame()) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

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

    std::vector<MeshDescriptor> mesh_ranges = std::vector<MeshDescriptor>();
    std::map<OpaqueAssetHandle, MeshDescriptor> mesh_descriptors = std::map<OpaqueAssetHandle, MeshDescriptor>();


    std::vector<std::pair<std::uint32_t,TextureRange>> mesh_to_albedo;



    PALADIN_LOG(INFO, "Offset Pos: " + std::to_string(offsetof(Paladin::Vertex, x)));
    PALADIN_LOG(INFO, "Offset Normal: " + std::to_string(offsetof(Paladin::Vertex, nx)));
    PALADIN_LOG(INFO, "Offset Tangent: " + std::to_string(offsetof(Paladin::Vertex, tx))); // Check this!
    PALADIN_LOG(INFO, "Offset Bitangent: " + std::to_string(offsetof(Paladin::Vertex, btx))); // Check this!
    PALADIN_LOG(INFO, "Offset Color: " + std::to_string(offsetof(Paladin::Vertex, r)));
    PALADIN_LOG(INFO, "Offset UV: " + std::to_string(offsetof(Paladin::Vertex, u)));

    //Load model from Generational Allocator using a Handle<T>
    auto sponza_model = m_asset_registry->GetAsset<ModelAsset>(sponza);

    auto sponza_model_two = m_asset_registry->GetAsset<ModelAsset>(sponza_two);



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

 for (int i = 0; i < sponza_model->meshes.size(); i++) {
     auto mesh_handle = sponza_model->meshes[i];
     auto mesh = m_asset_registry->GetAsset<MeshAsset>(mesh_handle);

     auto v_start = static_cast<std::uint32_t>(all_vertices.size());
     auto i_start = static_cast<std::uint32_t>(all_indices.size());

     all_indices.insert(all_indices.end(), mesh->indices.begin(), mesh->indices.end());
     all_vertices.insert(all_vertices.end(), mesh->vertices.begin(), mesh->vertices.end());

     auto material_handle = sponza_model->materials[sponza_model->mesh_to_material[i]];
     auto material = m_asset_registry->GetAsset<MaterialAsset>(material_handle);
     auto albedo_handle = material->GetTexture(Albedo);
     auto roughness_handle = material->GetTexture(Roughness);
     auto metallic_handle = material->GetTexture(Metallic);
     auto normal_handle = material->GetTexture(Normal);

     std::unique_ptr<GPUMaterial> gpu_material = std::make_unique<GPUMaterial>();
     gpu_material->albedo = {};
     gpu_material->normal = {};
     gpu_material->roughness = {};
     gpu_material->metallic= {};

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
            gpu_material->metallic = m_render_context->UploadTexture2D(*metallic_texture, metallic_handle);

     }
     auto normal_texture = m_asset_registry->GetAsset<Texture2DAsset>(normal_handle);
     if (normal_texture != nullptr) {
         gpu_material->normal= m_render_context->UploadTexture2D(*normal_texture, normal_handle);
     }

        m_render_context->AddMaterial(std::move(gpu_material),material_handle);

        auto material_indices = m_render_context->GetMaterialIndices(material_handle);
        auto mesh_descriptor = MeshDescriptor{
            .vertex_start_location = v_start,
            .index_start_location = i_start,
            .index_count = static_cast<std::uint32_t>(mesh->indices.size()),

            .albedo = material_indices.albedo,
            .normal = material_indices.normal,
            .roughness = material_indices.roughness,
            .metallic = material_indices.metallic,
        };
        auto mesh_descriptor_index = mesh_ranges.size();
        descriptor_fetch.insert(std::make_pair(mesh_handle,mesh_descriptor_index));
        mesh_ranges.push_back(mesh_descriptor);
    }

  for (int i = 0; i < sponza_model_two->meshes.size(); i++) {
     auto mesh_handle = sponza_model_two->meshes[i];
     auto mesh = m_asset_registry->GetAsset<MeshAsset>(mesh_handle);

     auto v_start = static_cast<std::uint32_t>(all_vertices.size());
     auto i_start = static_cast<std::uint32_t>(all_indices.size());

     all_indices.insert(all_indices.end(), mesh->indices.begin(), mesh->indices.end());
     all_vertices.insert(all_vertices.end(), mesh->vertices.begin(), mesh->vertices.end());

     auto material_handle = sponza_model_two->materials[sponza_model_two->mesh_to_material[i]];
     auto material = m_asset_registry->GetAsset<MaterialAsset>(material_handle);
     auto albedo_handle = material->GetTexture(Albedo);
     auto roughness_handle = material->GetTexture(Roughness);
     auto metallic_handle = material->GetTexture(Metallic);
     auto normal_handle = material->GetTexture(Normal);

     std::unique_ptr<GPUMaterial> gpu_material = std::make_unique<GPUMaterial>();
     gpu_material->albedo = {};
     gpu_material->normal = {};
     gpu_material->roughness = {};
     gpu_material->metallic= {};

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
            gpu_material->metallic = m_render_context->UploadTexture2D(*metallic_texture, metallic_handle);
     }
     auto normal_texture = m_asset_registry->GetAsset<Texture2DAsset>(normal_handle);
     if (normal_texture != nullptr) {
         gpu_material->normal= m_render_context->UploadTexture2D(*normal_texture, normal_handle);
     }

        m_render_context->AddMaterial(std::move(gpu_material),material_handle);

        auto material_indices = m_render_context->GetMaterialIndices(material_handle);
        auto mesh_descriptor = MeshDescriptor{
            .vertex_start_location = v_start,
            .index_start_location = i_start,
            .index_count = static_cast<std::uint32_t>(mesh->indices.size()),

            .albedo = material_indices.albedo,
            .normal = material_indices.normal,
            .roughness = material_indices.roughness,
            .metallic = material_indices.metallic,
        };
        auto mesh_descriptor_index = mesh_ranges.size();
        descriptor_fetch.insert(std::make_pair(mesh_handle,mesh_descriptor_index));
        mesh_ranges.push_back(mesh_descriptor);
    }

    Scene::SceneEntry entry = {};
    entry.model_handle = sponza;
    entry.transform_index = scene.all_transforms.size();
    entry.transform_count = 0;

    for (int x = 1; x < 15; x++) {
        for (int z =1; z < 15; z++) {
            auto transform = Transform{};
            transform.SetPosition({x*5000,0,z*5000});
            transform.SetScale({1,1,1});

            scene.all_transforms.push_back(transform);
            entry.transform_count++;
        }
    }
    scene.model_entries.push_back(entry);

    Scene::SceneEntry entry_two = {};
    entry_two.model_handle = sponza_two;
    entry_two.transform_index = scene.all_transforms.size();
    entry_two.transform_count = 0;
    for (int x = 1; x < 15; x++) {
        for (int z = 1; z < 15; z++) {
            auto transform = Transform{};
            transform.SetPosition({-x*5000,0,-z*5000});
            transform.SetScale({1,1,1});
            scene.all_transforms.push_back(transform);
            entry_two.transform_count++;
        }
    }
    scene.model_entries.push_back(entry_two);

    //m_render_context->UploadFrameData(m_camera);
    m_render_context->CreateInstanceBuffer();
    m_render_context->CreateVisibleInstanceIDBuffer();

    m_render_context->CreatePersistantAllocation(m_camera.GetUniformMut());
    m_render_context->UploadIVBuffers(all_vertices,all_indices);
    m_render_context->CreateIndirectCommandBuffer();
    m_render_context->UploadMeshDescriptors(mesh_ranges,descriptor_fetch);

}

bool RenderApplication::Update(float delta_time) {
    PALADIN_SCOPED_CPU_PROFILE("AppUpdate",ProfileColors::Blue);
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    m_camera.direction = glm::vec3(0.0);

    double mouse_x;
    double mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);


    glm::vec3 direction = glm::vec3(0.0);
    if (Input::keys[GLFW_KEY_W]) {
        direction.z =1;
    }
    if (Input::keys[GLFW_KEY_S]) {
        direction.z =-1;
    }
    if (Input::keys[GLFW_KEY_D]) {
        direction.x =1;
    }
    if (Input::keys[GLFW_KEY_A]) {
        direction.x=-1;
    }
    if (Input::keys[GLFW_KEY_R]) {
        direction.y=1;
    }
    if (Input::keys[GLFW_KEY_T]) {
       direction.y=-1;
    }


    if (Input::keys[GLFW_KEY_O]) {
        frustum_test = m_camera;
        frustum_test.direction = direction;
        frustum_test.FullUpdate(delta_time, mouse_x, mouse_y);
    }
    m_camera.direction =direction;
    m_camera.FullUpdate(delta_time, mouse_x, mouse_y);

    std::vector<ModelAsset*> models_to_draw;
    std::vector<DrawData> draw_batches;
    FrameUploadData test_upload_data = {};


    FrameUploadData frame_upload_data = {};
    frame_upload_data.camera = m_camera;
    std::unordered_map<AssetHandle<MeshAsset>, std::vector<std::uint32_t>> visible_instances;
    for (auto& entry : scene.model_entries) {
        auto model = m_asset_registry->GetAsset<ModelAsset>(entry.model_handle);
        auto model_tranforms = std::span(scene.all_transforms.data()+entry.transform_index,entry.transform_count);
        for (auto& transform : model_tranforms) {
            transform.Rotate(Axis::X_AXIS,0.5*delta_time);
            auto transform_data = transform.GetData();
            if (m_camera.IsOnFrustrum(model->bounding_box, transform.GetData())) {
                for (int i = 0; i < model->meshes.size(); i++) {
                    visible_instances[model->meshes[i]].push_back(frame_upload_data.transforms.size());
                }
                frame_upload_data.transforms.push_back(transform_data);

            }
        }
    }






    frame_upload_data.camera = m_camera;

    std::uint32_t triangle_count = 0;

    {
        PALADIN_SCOPED_CPU_PROFILE("GPU Extraction Write",ProfileColors::Blue);
        for (auto[mesh_handle, visible_instance_indices] : visible_instances) {
            if (visible_instance_indices.empty()) continue;
            FrameUploadData::MeshDrawBatch draw_batch ={};
            draw_batch.mesh_handle = mesh_handle;
            draw_batch.instance_count= visible_instance_indices.size();
            draw_batch.instance_index = frame_upload_data.instance_indices.size();
            frame_upload_data.instance_indices.insert(frame_upload_data.instance_indices.end(), visible_instance_indices.begin(), visible_instance_indices.end());
            frame_upload_data.visible_meshes.push_back(draw_batch);
            auto mesh = m_asset_registry->GetAsset<MeshAsset>(mesh_handle);
            triangle_count+=(mesh->indices.size()/3)*draw_batch.instance_count;
        }
    }


    ImGui::Begin("PerformanceStats");
    ImGui::Text("FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Rendered Triangles: %u", triangle_count);
    ImGui::Text("Rendered Vertices: %u", triangle_count*3);
    ImGui::End();

    m_render_context->UpdatePersistantAllocation(m_camera.GetUniformMut());
    m_render_context->UpdateFrameData(frame_upload_data);

    return true;
}

void RenderApplication::Render(float delta_time) {

}
