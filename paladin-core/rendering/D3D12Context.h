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



constexpr UINT FRAME_BUFFER_COUNT = 2;
class D3D12Context {
public:

    D3D12Context(HWND hwnd, std::uint32_t window_width, std::uint32_t window_height);
    ~D3D12Context();

private:


    Microsoft::WRL::ComPtr<ID3D12Device8> m_device = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swap_chain= nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_descriptor_heap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource2> m_render_target[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[FRAME_BUFFER_COUNT];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList8> m_command_list = nullptr;


    UINT frame_index;

};
#endif //PALADIN_D3DX12CONTEXT_H