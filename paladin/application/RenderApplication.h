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
#include "utility/StringHash.h"
#include <unordered_map>
#include <unordered_set>
#include "imgui_impl_glfw.h"
#include "Logger.h"
#include "Scene.h"
#include "asset-pipeline/AssetRegistry.h"
#include "asset/Texture2D.h"
#include "utility/GenIndices.h"
#include "data-structures/RingBuffer.h"


struct EntityTest {
    Transform transform;
    AssetHandle<ModelAsset> model_handle;
};

class Input {
public:
    inline static bool keys[1024]={false};
    static void InputCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
};




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

    std::shared_ptr<D3D12Context> m_render_context = nullptr;
    std::uint32_t window_width = 1920;
    std::uint32_t window_height = 1080;
    GLFWwindow* window = nullptr;

    Camera m_camera{};
    Camera frustum_test{};

    std::unique_ptr<AssetRegistry> m_asset_registry = nullptr;
    std::vector<Transform> transforms;
    std::vector<Transform> AABB_transforms;
    std::vector<Transform> AABB_real_transforms;
    std::vector<AABB> AABBs;
    std::vector<TransformData> instancing_test_data;

    float rot_test = 0.0;
    RenderFrameData frame_data{};
    float m_elapsed_time = 0.0f;
    float m_last_frame_time= 0.0f;


    //D3D12Context* m_render_context = nullptr;
};
#endif //PALADIN_RENDERAPPLICATION_H
