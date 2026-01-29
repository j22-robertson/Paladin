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
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }



class D3D12Context {
public:

    D3D12Context(HWND hwnd, std::uint32_t window_width, std::uint32_t window_height);
    ~D3D12Context();

private:
    std::uint32_t frame_buffer_count = 2;


    Microsoft::WRL::ComPtr<ID3D12Device8> m_device = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue = nullptr;

};
#endif //PALADIN_D3DX12CONTEXT_H