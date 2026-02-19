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

#include "Logger.h"
#include "Vertex.h"
#include "asset/Mesh.h"

struct Vertex {
    Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, a) {}
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
};


constexpr UINT FRAME_BUFFER_COUNT = 3;
class D3D12Context {
public:

    D3D12Context(HWND hwnd, std::uint32_t window_width, std::uint32_t window_height);

    Microsoft::WRL::ComPtr<ID3D12Resource> UploadVertices();

    bool Render();

    void OnResize(std::uint32_t new_width, std::uint32_t new_height) {
        m_window_width = new_width;
        m_window_height = new_height;
        m_resized = true;
    }

    void UploadModel(std::span<Paladin::Vertex> model_vertices, std::span<std::uint32_t> model_indices, std::span<MeshRange> mesh_ranges);

    bool m_resized = false;
    ~D3D12Context();

private:
    bool WaitForPreviousFrame();
    bool UpdatePipeline();
    void FlushDevice();
    bool Resize(std::uint32_t new_width, std::uint32_t new_height);
    Microsoft::WRL::ComPtr<ID3D12Device8> m_device = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swap_chain= nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_descriptor_heap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srv_descriptor_heap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource2> m_render_target[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList8> m_command_list = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline_state = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> triangle_vertex_buffer = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> temporary_upload_heap= nullptr;


    Microsoft::WRL::ComPtr<D3D12MA::Allocator> m_gpu_allocator = nullptr;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter = nullptr;

    std::uint32_t m_window_width;
    std::uint32_t m_window_height;

    DXShader vertex_shader;
    DXShader fragment_shader;

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view ={};
    Vertex triangle_vertices[3] = {
        { 0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },
        { -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
    };


    DXShaderCompiler m_shader_compiler = DXShaderCompiler();

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