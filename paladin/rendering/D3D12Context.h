//
// Created by James Robertson on 29/01/2026.
//

#ifndef PALADIN_D3DX12CONTEXT_H
#define PALADIN_D3DX12CONTEXT_H
#include <d3dx12/d3dx12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgi.h>
#include "D3D12MemAlloc.h"
#include "debuglayer/D3D12DebugLayer.h"
#include "DXShaderCompiler.h"
#include "DirectXMath.h"
#include "imgui_impl_dx12.h"
#include  "imgui.h"
#include "profiling/Profiling.h"
#include <filesystem>
#include <shlobj.h>
#include <span>

#include "Camera.h"
#include "Logger.h"
#include "Transform.h"
#include "Vertex.h"
#include "asset/Mesh.h"
#include "asset/Texture2D.h"
#include "render-utils/GraphicsPipelineState.h"
#include "resource/GPUResourceRegistry.h"
#include "resource/ResourceManager.h"
#include "ForwardPass.h"
#include "GPUBufferEntry.h"
#include "descriptor/DescriptorHeap.h"
#include "descriptor/DescriptorHeapManager.h"

struct Vertex {
    Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, a) {}
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
};

struct MeshData {
    std::uint32_t material_id;
};

struct DrawData {
    std::uint32_t mesh_index = -1;
    std::uint32_t albedo = -1;
    std::vector<std::uint32_t> transform_indices;
};



struct SceneFrameData {
    std::uint32_t camera_identifier;
    std::uint32_t frame_index;
    float delta_time;

    /*
    std::uint32_t lights_identifier;
    std::uint32_t environment_map;*/
};




constexpr UINT FRAME_BUFFER_COUNT = 3;
constexpr UINT MAX_INSTANCES = 100000;
class D3D12Context {
public:

    D3D12Context(HWND hwnd, std::uint32_t window_width, std::uint32_t window_height);
    bool Render();
    std::optional<std::pair<Microsoft::WRL::ComPtr<D3D12MA::Allocation>, Microsoft::WRL::ComPtr<D3D12MA::Allocation>>>
    CreateAllocation(
        const D3D12_RESOURCE_DESC& resource_desc, void* data);

    void CreateVisibleInstanceIDBuffer();

    void CrateVisibleInstanceIDBuffer();

    void CreatePersistantAllocation(CameraUniform uniform_data);
    void UpdatePersistantAllocation(CameraUniform uniform_data);

    void CreateInstanceBuffer();
    void UpdateInstanceBufferT(std::span<TransformData> instance_data);


    void UpdateFrameData(FrameUploadData &render_frame_data);

    void UpdateRenderFrameData(RenderFrameData& render_frame_data);
    void OnResize(std::uint32_t new_width, std::uint32_t new_height) {
        m_window_width = new_width;
        m_window_height = new_height;
        m_resized = true;
    }


    GPUResourceHandle<GPUTexture2D>  UploadTexture2D( Texture2DAsset& texture, OpaqueAssetHandle handle);
    GPUResourceHandle<GPUMesh> UploadMesh(MeshAsset& mesh,OpaqueAssetHandle handle);

    GPUResourceHandle<GPUMaterial> AddMaterial(std::unique_ptr<GPUMaterial> material, OpaqueAssetHandle handle);
    GPUResourceHandle<GPUModel> AddModel(std::unique_ptr<GPUModel> model, OpaqueAssetHandle handle);

    OpaqueAssetHandle model_handle;

    RenderFrameData _frame_data;

    void UploadModel(std::span<Paladin::Vertex> model_vertices,
        std::span<std::uint32_t> model_indices,
        std::vector<MeshRange> mesh_ranges);


    bool m_resized = false;
    int imgui_descriptor_index=0;
    ~D3D12Context();
    bool WaitForPreviousFrame();
private:

    bool UpdatePipeline();
    void FlushDevice();
    bool Resize(std::uint32_t new_width, std::uint32_t new_height);
    Microsoft::WRL::ComPtr<ID3D12Device8> m_device = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swap_chain= nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_descriptor_heap = nullptr;
    //Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srv_descriptor_heap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsv_descriptor_heap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_imgui_srv_descriptor_heap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource2> m_render_target[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList8> m_command_list = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline_state = nullptr;
    //Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_bindless_root_signature = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> triangle_vertex_buffer = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> temporary_upload_heap= nullptr;

    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> m_depth_buffer;

    std::vector<std::pair<D3D12_VERTEX_BUFFER_VIEW,Microsoft::WRL::ComPtr<D3D12MA::Allocation>>> vertex_buffers;

    std::vector<std::pair<D3D12_INDEX_BUFFER_VIEW, Microsoft::WRL::ComPtr<D3D12MA::Allocation>>> index_buffers;

    std::vector<std::vector<MeshRange>> model_mesh_ranges;

    UINT custom_rtv_target_index = 0;

    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> custom_targets;

    std::unique_ptr<GraphicsPipelineState> test_state = nullptr;

    std::unique_ptr<GraphicsPipelineState> aabb_state = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> aabb_pipeline = nullptr;
    DXShader aabb_vs;
    DXShader aabb_fs;
   // std::pair<Microsoft::WRL::ComPtr<D3D12MA::Allocation>,Microsoft::WRL::ComPtr<D3D12MA::Allocation>> m_camera_allocation;

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_frame_data=nullptr;
    void* frame_data_destination = nullptr;


    DescriptorHeapManager m_descriptor_heap;

    UINT frame_descriptor_start = 0;

    Microsoft::WRL::ComPtr<D3D12MA::Allocator> m_gpu_allocator = nullptr;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter = nullptr;

    GPUResourceRegistry m_gpu_resources = GPUResourceRegistry();
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_instance_id_data=nullptr;
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> m_instance_data=nullptr;
    void* instance_data_destination = nullptr;
    void* id_data_destination = nullptr;
    //std::uint32_t instance_buffer_index;
    std::uint32_t instance_count;
    //std::uint32_t bytes_per_buffer;
    //std::uint32_t aligned_bytes_per_buffer;

    FrameUploadData temp_frame_data;


    std::uint32_t temp_aligned_byes;

    std::uint32_t m_window_width;
    std::uint32_t m_window_height;

    std::map<std::uint32_t, DescriptorHeap> m_descriptor_heaps;


    DXShader vertex_shader;
    DXShader fragment_shader;
    DXShader test_vs;

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view ={};
    Vertex triangle_vertices[3] = {
        { 0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },
        { -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
    };




    DXShaderCompiler m_shader_compiler = DXShaderCompiler();

    ViewHandle depth_views[3];
    ViewHandle custom_rt_srv[3];
    ViewHandle camera_cbv;

    ViewHandle instance_buffer_view;
    GPUBufferEntry instance_buffer_entry;

    ViewHandle instance_id_view;
    GPUBufferEntry instance_draw_id_entry;

    D3D12_VIEWPORT m_viewport = {};
    D3D12_RECT m_scissor = {};

    HANDLE fence_event = nullptr;


    UINT fence_value[FRAME_BUFFER_COUNT] = {};
    UINT frame_index;

#ifdef TRACY_ENABLE
    tracy::D3D12QueueCtx* m_tracy_context = nullptr;
#endif

};


#endif //PALADIN_D3DX12CONTEXT_H
