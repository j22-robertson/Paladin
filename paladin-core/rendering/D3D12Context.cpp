//
// Created by James Robertson on 29/01/2026.
//
#include "D3D12Context.h"

#include <iostream>
#include <ostream>

D3D12Context::D3D12Context(HWND hwnd, std::uint32_t window_width, std::uint32_t window_height) {
#ifdef _DEBUG
    D3D12DebugLayer::Get().Init();
    UINT factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#else
    UINT factory_flags = 0;
#endif

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter = nullptr;

    Microsoft::WRL::ComPtr<IDXGIFactory2> m_dxgi_factory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory2(factory_flags,IID_PPV_ARGS(&m_dxgi_factory)))) {
        std::cout << "Created DXGIFactory" << std::endl;
    }

    bool adapter_found = false;
    for (UINT adapter_index= 0; m_dxgi_factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND; adapter_index++ )
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr))) {
            std::wcout << desc.Description << std::endl;
            adapter_found = true;
            break;
        }
    }

    if (adapter_found) {
        std::cout << "Found DXGI device adapter" << std::endl;
        if (const auto hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)); SUCCEEDED(hr))
        {
            if (m_device.Get()) {
                std::cout << "Created D3D12 device" << std::endl;
            }
            std::cout << "Created device" << std::endl;
        }
        else if (FAILED(hr)){
            std::cout << "Failed to create device. Error code: " << std::hex << hr << std::endl;
            return;
        }
    }

    D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
    command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (auto hr = m_device->CreateCommandQueue(&command_queue_desc,IID_PPV_ARGS(&m_command_queue)); SUCCEEDED(hr)) {
        std::cout << "Created command queue" << std::endl;
    }
    else if (FAILED(hr)) {
        std::cout << "Failed to create command queue. Error code: " << std::hex << hr << std::endl;
    }



    DXGI_MODE_DESC backbuffer_desc = {};
    backbuffer_desc.Width = window_width;
    backbuffer_desc.Height = window_height;
    backbuffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


    DXGI_SAMPLE_DESC sample_desc{};
    sample_desc.Count = 1;
    sample_desc.Quality=0;

    DXGI_SWAP_CHAIN_DESC swap_chain_desc = { };

    swap_chain_desc.BufferCount = frame_buffer_count;
    swap_chain_desc.BufferDesc = backbuffer_desc;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.OutputWindow = hwnd;
    swap_chain_desc.SampleDesc = sample_desc;
    swap_chain_desc.Windowed = true;

    Microsoft::WRL::ComPtr<IDXGISwapChain> temp_swap_chain;

    if (auto hr =m_dxgi_factory->CreateSwapChain(
        m_command_queue.Get(),
        &swap_chain_desc,
        &temp_swap_chain
    ); SUCCEEDED(hr)) {
        std::cout << "Created swap chain successfully" << std::endl;
    }
    else if (FAILED(hr)) {
        std::cout << "Failed to create swap chain. ERROR: "<<std::hex<<hr << std::endl;
    }

    


}

D3D12Context::~D3D12Context() {
#ifdef _DEBUG
    D3D12DebugLayer::Get().Shutdown();
#endif

}
