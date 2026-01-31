//
// Created by James Robertson on 29/01/2026.
//

#ifndef PALADIN_D3DX12CONTEXT_H
#define PALADIN_D3DX12CONTEXT_H
#include <d3dx12/d3dx12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgi.h>
#include "debuglayer/D3D12DebugLayer.h"
#include "DXShaderCompiler.h"

#include <filesystem>
#include <shlobj.h>
static std::wstring GetLatestWinPixGpuCapturerPath_Cpp17()
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    std::filesystem::path pixInstallationPath = programFilesPath;
    pixInstallationPath /= "Microsoft PIX";

    std::wstring newestVersionFound;

    for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
    {
        if (directory_entry.is_directory())
        {
            if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
            {
                newestVersionFound = directory_entry.path().filename().c_str();
            }
        }
    }

    if (newestVersionFound.empty())
    {
        return L"No pix installation found";
    }

    return pixInstallationPath/ newestVersionFound / L"WinPixGpuCapturer.dll";
}

constexpr UINT FRAME_BUFFER_COUNT = 2;
class D3D12Context {
public:

    D3D12Context(HWND hwnd, std::uint32_t window_width, std::uint32_t window_height);

    bool Render();

    ~D3D12Context();

private:
    bool WaitForPreviousFrame();
    bool UpdatePipeline();

    Microsoft::WRL::ComPtr<ID3D12Device8> m_device = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swap_chain= nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_descriptor_heap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource2> m_render_target[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList8> m_command_list = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline_state = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature = nullptr;

    DXShaderCompiler m_shader_compiler = DXShaderCompiler();

    D3D12_VIEWPORT m_viewport = {};
    D3D12_RECT m_scissor = {};

    HANDLE fence_event = nullptr;

    UINT fence_value[FRAME_BUFFER_COUNT] = {};
    UINT frame_index;

};
#endif //PALADIN_D3DX12CONTEXT_H