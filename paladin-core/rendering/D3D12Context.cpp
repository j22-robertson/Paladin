//
// Created by James Robertson on 29/01/2026.
//
#include "D3D12Context.h"

#include <dxcapi.h>
#include <iostream>
#include <ostream>

D3D12Context::D3D12Context(HWND hwnd, std::uint32_t window_width, std::uint32_t window_height) {

#ifdef _DEBUG

    if (GetModuleHandle(reinterpret_cast<LPCSTR>(L"WinPixGpuCapturer.dll")) == 0)
    {

        //LPCSTR pix_path = GetLatestWinPixGpuCapturerPath_Cpp17().c_str();
        std::wcout << "loading pix from: " << GetLatestWinPixGpuCapturerPath_Cpp17() << std::endl;
        //LoadLibrary();
        LoadLibraryW(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
    }
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
    bool able_to_support_feature = true;
    for (UINT adapter_index= 0; able_to_support_feature; adapter_index++ )
    {
        if (auto hr =  m_dxgi_factory->EnumAdapters1(adapter_index, &adapter); FAILED(hr)) {
            std::cout << "Failed to match feature level. Error code: " << std::hex << hr << std::endl;
            able_to_support_feature = false;
        }

        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, _uuidof(ID3D12Device), nullptr))) {
            std::wcout << desc.Description << std::endl;
            adapter_found = true;
            break;
        }
    }

    if (adapter_found) {
        std::cout << "Found DXGI device adapter" << std::endl;
        if (const auto hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_device)); SUCCEEDED(hr))
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

    swap_chain_desc.BufferCount = FRAME_BUFFER_COUNT;
    swap_chain_desc.BufferDesc = backbuffer_desc;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.OutputWindow = hwnd;
    swap_chain_desc.SampleDesc = sample_desc;
    swap_chain_desc.Windowed = true;


    if (auto hr =m_dxgi_factory->CreateSwapChain(
        m_command_queue.Get(),
        &swap_chain_desc,
        reinterpret_cast<IDXGISwapChain**>(m_swap_chain.GetAddressOf())
    ); SUCCEEDED(hr)) {
        std::cout << "Created swap chain successfully" << std::endl;
    }
    else if (FAILED(hr)) {
        std::cout << "Failed to create swap chain. ERROR: "<<std::hex<<hr << std::endl;
    }

    frame_index = m_swap_chain->GetCurrentBackBufferIndex();

    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};

    rtv_heap_desc.NumDescriptors = FRAME_BUFFER_COUNT;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (auto hr = m_device->CreateDescriptorHeap(&rtv_heap_desc,IID_PPV_ARGS(&m_rtv_descriptor_heap)); SUCCEEDED(hr)) {
        std::cout << "Created rtv descriptor heap successfully" << std::endl;
    }
    else if (FAILED(hr)) {
        std::cout << "Failed to create descriptor heap. ERROR: "<<std::hex<<hr << std::endl;
    }

    auto rtv_descriptor_size =  m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle(m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        if (auto hr = m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_render_target[i])); SUCCEEDED(hr)) {
            std::cout << "Created render target." << std::endl;
        }
        else if (FAILED(hr)) {
            std::cout << "Failed to get render target. ERROR: "<<std::hex<<hr << std::endl;
        }
        m_device->CreateRenderTargetView(m_render_target[i].Get(), nullptr, rtv_cpu_handle);
        rtv_cpu_handle.ptr += rtv_descriptor_size;
    }

    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        if (auto hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator[i])); SUCCEEDED(hr)) {
            std::cout << "Created command allocator" << std::endl;
        }
        else if (FAILED(hr)) {
            std::cout << "Failed to create Commannd Allocator. ERROR: "<<std::hex<<hr << std::endl;
        }
    }

    if (auto hr = m_device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,m_command_allocator[0].Get(),NULL, IID_PPV_ARGS(&m_command_list)); SUCCEEDED(hr)) {
        std::cout << "Created Command List." << std::endl;
    }
    else if (FAILED(hr)) {
        std::cout << "Failed to create Command List. ERROR: "<<std::hex<<hr << std::endl;
    }
    m_command_list->Close();

    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++)
    {
        if (auto hr = m_device->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&m_fence[i])); SUCCEEDED(hr))
        {
            std::cout << "Created Fence." << std::endl;
        }
        else if (FAILED(hr))
        {
            std::cout << "Failed to create Fence. ERROR: "<<std::hex<<hr << std::endl;
        }
        fence_value[i] = 0;
    }


    HANDLE fence_event = CreateEvent(nullptr, false, false, nullptr);

    if (fence_event == nullptr)
    {
        std::cout << "Unable to create fence event" << std::endl;
    }

    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.Init(0,nullptr,0,nullptr,D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signature = nullptr;

    if (auto hr = D3D12SerializeRootSignature(&root_signature_desc,D3D_ROOT_SIGNATURE_VERSION_1_0,&signature,nullptr); SUCCEEDED(hr))
    {
        std::cout << "Succesfully serialized root signature" << std::endl;
    }
    else if (FAILED(hr))
    {
        std::cout << "Failed to serialize root signature. ERROR: "<<std::hex<<hr << std::endl;
        return;
    }

    if (auto hr = m_device->CreateRootSignature(0,signature->GetBufferPointer(),signature->GetBufferSize(),IID_PPV_ARGS(&m_root_signature)); SUCCEEDED(hr))
    {
        std::cout << "Succesfully created root signature" << std::endl;
    }
    else if (FAILED(hr))
    {
        std::cout << "Failed to create root signature. ERROR: "<<std::hex<<hr << std::endl;
    }
    Microsoft::WRL::ComPtr<ID3DBlob> vs_blob = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> vs_blob_encoding = nullptr;


    /*
    Microsoft::WRL::ComPtr<IDxcLibrary> m_library = nullptr;
    if (auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library)); FAILED(hr))
    {
        std::cout << "Unable to create DxcLibrary ERROR: " << std::hex<<hr << std::endl;
    }
    Microsoft::WRL::ComPtr<IDxcCompiler3> m_compiler = nullptr;
    if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)); FAILED(hr))
    {
        std::cout << "Unable to create DxcCompiler. ERROR: " << std::hex<<hr << std::endl;
    }


    if (auto hr = m_library->CreateBlobFromFile(L"../paladin-core/rendering/shaders/vs.hlsl",&code_page,vs_blob_encoding.GetAddressOf()); FAILED(hr))
    {
        std::cout << "Unable to create source blob for vs.hlsl ERROR: " << std::hex<<hr << std::endl;
    }*/
    std::filesystem::path root = PALADIN_ROOT_DIR;

    std::filesystem::path shader = root / "paladin-core/rendering/shaders/vs.hlsl";

    if (std::filesystem::exists(shader))
    {
        std::cout << shader << std::endl;
    }
    else
    {
        std::cout << "unable to find path" << std::endl;
    }

    auto vertex_shader = m_shader_compiler.LoadShader(shader);

    //TODO: Clean up shader compilation code into a DXCompiler class


}

D3D12Context::~D3D12Context() {
#ifdef _DEBUG
    D3D12DebugLayer::Get().Shutdown();
#endif

}
