//
// Created by James Robertson on 29/01/2026.
//
#include "D3D12Context.h"

#include <dxcapi.h>
#include <iostream>
#include <ostream>

#include "GPUBufferEntry.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"


D3D12Context::D3D12Context(HWND hwnd, std::uint32_t window_width, std::uint32_t window_height) {
#ifdef PIX_ENABLE
    LoadWinPixEventRuntime();
#endif
#ifdef _DEBUG
    D3D12DebugLayer::Get().Init();
    UINT factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#else
    UINT factory_flags = 0;
#endif

    m_window_width = window_width;
    m_window_height = window_height;



    Microsoft::WRL::ComPtr<IDXGIFactory2> m_dxgi_factory = nullptr;
    if (auto hr = CreateDXGIFactory2(factory_flags,IID_PPV_ARGS(&m_dxgi_factory)); FAILED(hr)) {
        PALADIN_LOG(INFO, "Created DXGI Factory")
    }

    bool adapter_found = false;
    bool able_to_support_feature = true;
    for (UINT adapter_index= 0; able_to_support_feature; adapter_index++ )
    {
        if (auto hr =  m_dxgi_factory->EnumAdapters1(adapter_index, &m_adapter); FAILED(hr)) {
            PALADIN_LOG(ERR, ErrorResult("Failed to match feature level." , hr))
            able_to_support_feature = false;
        }

        DXGI_ADAPTER_DESC1 desc;
        m_adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_2, _uuidof(ID3D12Device), nullptr))) {
            std::wstring gpu_name = desc.Description;
            PALADIN_LOG(INFO, "Found Graphics Device: " + ConvertWString(gpu_name))
            adapter_found = true;
            break;
        }
    }

    if (adapter_found) {
        PALADIN_LOG(INFO, "Found DXGI Device Adapter")
        if (const auto hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_device)); SUCCEEDED(hr))
        {
            if (m_device.Get()) {
                PALADIN_LOG(INFO, "Created D3D12 Device")
            }
        }
        else if (FAILED(hr)){
            PALADIN_LOG(ERR, ErrorResult("Failed to create device.",hr))
            return;
        }
    }


    D3D12MA::ALLOCATOR_DESC allocator_desc = {};
    allocator_desc.pDevice = m_device.Get();
    allocator_desc.pAdapter = m_adapter.Get();
    allocator_desc.Flags = D3D12MA_RECOMMENDED_ALLOCATOR_FLAGS;


    if (auto hr = D3D12MA::CreateAllocator(&allocator_desc, &m_gpu_allocator); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create D3D12MA::Allocator", hr))
    }
    else
    {
        PALADIN_LOG(INFO, "Created D3D12 Allocator")
    }



    D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
    command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (auto hr = m_device->CreateCommandQueue(&command_queue_desc,IID_PPV_ARGS(&m_command_queue)); SUCCEEDED(hr)) {
        PALADIN_LOG(INFO, "Created command queue")
    }
    else if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create command queue", hr))
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
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;



    if (auto hr =m_dxgi_factory->CreateSwapChain(
        m_command_queue.Get(),
        &swap_chain_desc,
        reinterpret_cast<IDXGISwapChain**>(m_swap_chain.GetAddressOf())
    ); SUCCEEDED(hr)) {
        PALADIN_LOG(INFO, "Successfully created swap chain")
    }
    else if (FAILED(hr)) {
        PALADIN_LOG(ERR,ErrorResult("Failed to create swap chain",hr))
    }

    frame_index = m_swap_chain->GetCurrentBackBufferIndex();

    D3D12_DESCRIPTOR_HEAP_DESC imgui_srv_heap_desc = {};

    //TODO: Change num descriptors
    imgui_srv_heap_desc.NumDescriptors =512;
    imgui_srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    imgui_srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (auto hr = m_device->CreateDescriptorHeap(&imgui_srv_heap_desc,IID_PPV_ARGS(&m_imgui_srv_descriptor_heap)); SUCCEEDED(hr)) {
        PALADIN_LOG(INFO, "Created imgui srv descriptor heap")
    }
    else if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create imgui srv descriptor heap", hr))
    }



    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};


    //TODO: Change num descriptors
    srv_heap_desc.NumDescriptors =100000;
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
/*
    if (auto hr = m_device->CreateDescriptorHeap(&srv_heap_desc,IID_PPV_ARGS(&m_srv_descriptor_heap)); SUCCEEDED(hr)) {
        PALADIN_LOG(INFO, "Created srv descriptor heap")
    }
    else if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create srv descriptor heap", hr))
    }*/

    if (m_descriptor_heap.CreateDescriptorHeap(m_device.Get(),PALADIN_HASH("BINDLESS"),srv_heap_desc))
    {
        PALADIN_LOG(INFO, "Successfully created BINDLESS")
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};

    rtv_heap_desc.NumDescriptors = FRAME_BUFFER_COUNT*2;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (auto hr = m_device->CreateDescriptorHeap(&rtv_heap_desc,IID_PPV_ARGS(&m_rtv_descriptor_heap)); SUCCEEDED(hr)) {
        PALADIN_LOG(INFO, "Successfully created rtv descriptor heap")
    }
    else if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create rtv descriptor heap", hr))
    }

    auto rtv_descriptor_size =  m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    auto srv_descriptor_size =  m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle(m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    //D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle(m_srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());


    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    auto e = 0;
    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        if (auto hr = m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_render_target[i])); SUCCEEDED(hr)) {
            PALADIN_LOG(INFO, "Created render target")
        }
        else if (FAILED(hr)) {
            PALADIN_LOG(ERR, ErrorResult("Failed to create render target", hr))
        }
        m_device->CreateRenderTargetView(m_render_target[i].Get(), nullptr, rtv_cpu_handle);

        rtv_cpu_handle.ptr += rtv_descriptor_size;
        e+=1;

    }


    D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
    depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depth_stencil_view_desc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_RESOURCE_DESC depth_stencil_desc = {};
    depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depth_stencil_desc.Width = window_width;
    depth_stencil_desc.Height = window_height;
    depth_stencil_desc.MipLevels = 1;
    depth_stencil_desc.DepthOrArraySize = 1;
    depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    depth_stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_stencil_desc.SampleDesc = sample_desc;

    D3D12_CLEAR_VALUE depth_optimized_clear_value = {};

    depth_optimized_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    depth_optimized_clear_value.DepthStencil.Depth = 1.0f;
    depth_optimized_clear_value.DepthStencil.Stencil = 0;


    D3D12MA::ALLOCATION_DESC allocation_desc = {};
    allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
        Microsoft::WRL::ComPtr<D3D12MA::Allocation> depth_allocation = nullptr;
        auto hr = m_gpu_allocator->CreateResource(&allocation_desc,
             &depth_stencil_desc,
             D3D12_RESOURCE_STATE_DEPTH_WRITE,
             &depth_optimized_clear_value,
             &depth_allocation,
             IID_NULL,
             nullptr);
        if (FAILED(hr)) {
            PALADIN_LOG(ERR, ErrorResult("Failed to create depth stencil allocation", hr))
        }
        m_depth_buffer.push_back(std::move(depth_allocation));
    }



    D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};

    dsv_heap_desc.NumDescriptors = FRAME_BUFFER_COUNT;
    dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    m_descriptor_heap.CreateDescriptorHeap(m_device.Get(),PALADIN_HASH("DEPTH"),dsv_heap_desc);
    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        depth_views[i]=m_descriptor_heap.CreateDepthStencilView(m_device.Get(), PALADIN_HASH("DEPTH"),m_depth_buffer[i]->GetResource());
    }


    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        if (auto hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator[i])); SUCCEEDED(hr)) {
            PALADIN_LOG(INFO, "Created command allocator")
        }
        else if (FAILED(hr)) {
            PALADIN_LOG(ERR, ErrorResult("Failed to create command allocator", hr))

        }
    }

    if (auto hr = m_device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,m_command_allocator[0].Get(),NULL, IID_PPV_ARGS(&m_command_list)); SUCCEEDED(hr)) {
        PALADIN_LOG(INFO, "Created Command List")
    }
    else if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create command list", hr))
    }

    // m_command_list->Close();

    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++)
    {
        if (auto hr = m_device->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&m_fence[i])); SUCCEEDED(hr))
        {
            PALADIN_LOG(INFO, "Created Fence.")

        }
        else if (FAILED(hr))
        {
            PALADIN_LOG(ERR, ErrorResult("Failed to create Fence", hr))
        }
        fence_value[i] = 0;
    }


    fence_event = CreateEvent(nullptr, false, false, nullptr);
    if (fence_event == nullptr)
    {
        PALADIN_LOG(ERR, "Failed to create fence event.")

        return;
    }







    D3D12_VERSIONED_ROOT_SIGNATURE_DESC bindless_root_signature_desc = {};

    D3D12_DESCRIPTOR_RANGE1 srv_ranges[1] ={};

    srv_ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srv_ranges[0].NumDescriptors = 3000;
    srv_ranges[0].BaseShaderRegister = 0;
    srv_ranges[0].RegisterSpace = 0;
    srv_ranges[0].OffsetInDescriptorsFromTableStart = 0;
    srv_ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

    D3D12_ROOT_PARAMETER1 bindless_params[5];
    D3D12_ROOT_PARAMETER1 bindless_root_parameter{};
    bindless_root_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    bindless_root_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    bindless_root_parameter.DescriptorTable.pDescriptorRanges = &srv_ranges[0];
    bindless_root_parameter.DescriptorTable.NumDescriptorRanges = 1;

    D3D12_ROOT_PARAMETER1 b_camera_root_parameter{};
    b_camera_root_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    b_camera_root_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    b_camera_root_parameter.Descriptor.RegisterSpace = 0;
    b_camera_root_parameter.Descriptor.ShaderRegister = 0;

    D3D12_ROOT_PARAMETER1 mesh_constant_parameter{};

    mesh_constant_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    mesh_constant_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    mesh_constant_parameter.Constants.ShaderRegister = 1;
    mesh_constant_parameter.Constants.RegisterSpace = 0;
    mesh_constant_parameter.Constants.Num32BitValues = 1;

    D3D12_ROOT_PARAMETER1 instance_constant_parameter{};
    instance_constant_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    instance_constant_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    instance_constant_parameter.Constants.ShaderRegister = 2;
    instance_constant_parameter.Constants.RegisterSpace = 0;
    instance_constant_parameter.Constants.Num32BitValues = 1;

    D3D12_ROOT_PARAMETER1 frame_data_parameter{};
    frame_data_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    frame_data_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    frame_data_parameter.Constants.ShaderRegister = 3;
    frame_data_parameter.Constants.RegisterSpace = 0;
    frame_data_parameter.Constants.Num32BitValues = 8;


    bindless_params[0] = b_camera_root_parameter;
    bindless_params[1] = bindless_root_parameter;
    bindless_params[2] = mesh_constant_parameter;
    bindless_params[3] = instance_constant_parameter;
    bindless_params[4] = frame_data_parameter;

    CD3DX12_STATIC_SAMPLER_DESC samplers[1] ={};
    samplers[0].Init(0,D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

    bindless_root_signature_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    bindless_root_signature_desc.Desc_1_1.NumParameters = 5;
    bindless_root_signature_desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

    //bindless_root_signature_desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED| D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    bindless_root_signature_desc.Desc_1_1.pParameters = bindless_params;
    bindless_root_signature_desc.Desc_1_1.NumStaticSamplers = 1;
    bindless_root_signature_desc.Desc_1_1.pStaticSamplers = samplers;

    Microsoft::WRL::ComPtr<ID3DBlob> bindless_signature = nullptr;

    if (auto hr = D3D12SerializeVersionedRootSignature(&bindless_root_signature_desc,&bindless_signature,nullptr); SUCCEEDED(hr))
    {
        PALADIN_LOG(INFO, "Successfully serialized bindless root signature")
    }
    else if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to serialize binless root signature",hr))
        return;
    }

    if (auto hr = m_device->CreateRootSignature(0,bindless_signature->GetBufferPointer(),bindless_signature->GetBufferSize(),IID_PPV_ARGS(&m_bindless_root_signature)); SUCCEEDED(hr))
    {
        PALADIN_LOG(INFO, "Successfully created bindless root signature");
    }
    else if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create bindless root signature", hr))
    }



    D3D12_INDIRECT_ARGUMENT_DESC arguments[3] = {};
    arguments[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    arguments[0].Constant.RootParameterIndex = 2;
    arguments[0].Constant.Num32BitValuesToSet = 1;
    arguments[0].Constant.DestOffsetIn32BitValues = 0;

    arguments[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
    arguments[1].Constant.RootParameterIndex = 3;
    arguments[1].Constant.Num32BitValuesToSet = 1;
    arguments[1].Constant.DestOffsetIn32BitValues = 0;

    arguments[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

    D3D12_COMMAND_SIGNATURE_DESC command_signature_desc = {};
    command_signature_desc.ByteStride = sizeof(InstancedIndirectCmd);
    command_signature_desc.NumArgumentDescs = 3;
    command_signature_desc.pArgumentDescs = arguments;
    command_signature_desc.NodeMask = 0;


    if (auto hr = m_device->CreateCommandSignature(
    &command_signature_desc,
    m_bindless_root_signature.Get(),
    IID_PPV_ARGS(&m_command_signature));
    FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Unable to create command signature",hr))
    }

    vertex_shader = DXShader(VertexShader, L"vs.vert",L"main");
    D3D12_SHADER_BYTECODE vertex_shader_bytecode = {};


    if (m_shader_compiler.LoadShader(vertex_shader))
    {
        vertex_shader_bytecode = vertex_shader.GetBytecode();
        PALADIN_LOG(INFO, "Loaded: " + ConvertWString(vertex_shader.shader_input_file) )
    }
    else
    {
        PALADIN_LOG(ERR, "Unable to load: " + ConvertWString(vertex_shader.shader_input_file))
    }

    D3D12_SHADER_BYTECODE fragment_shader_bytecode = {};
    //fragment_shader = DXShader(FragmentShader, L"fs.frag",L"main");
    fragment_shader = DXShader(FragmentShader, L"fragment_vpull.hlsl",L"main");

    if (!m_shader_compiler.LoadShader(fragment_shader))
    {
        PALADIN_LOG(ERR, "Unable to load: " + ConvertWString(fragment_shader.shader_input_file))
        return;
    }

    fragment_shader_bytecode = fragment_shader.GetBytecode();



    //test_vs = DXShader(VertexShader, L"test_vs.hlsl", L"main");
    test_vs = DXShader(VertexShader, L"vertexpull.hlsl", L"main");

    if (m_shader_compiler.LoadShader(test_vs))
    {
        PALADIN_LOG(INFO, "Successfully loaded test shader")
    }

   vertex_shader_bytecode = test_vs.GetBytecode();

    test_state = std::make_unique<PipelineState>(test_vs,fragment_shader,m_bindless_root_signature.Get());
    test_state->gpu_pso.InputLayout = {nullptr, 0};
    //test_state->input_layout_desc = {nullptr, 0};

    int channels = 4;
    std::vector<float> blank_data;
    blank_data.resize(window_width*window_height*channels);

    auto viewport_srv_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32G32B32A32_FLOAT,
        window_width,
        window_height,
        1,
        1,
        1,
        0,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,D3D12_TEXTURE_LAYOUT_UNKNOWN,
        0);

    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> upload_allocations;
    custom_rtv_target_index = e;
    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        if (auto allocation_pair = CreateAllocation(viewport_srv_desc,blank_data.data());allocation_pair.has_value()) {

            auto [custom_rendertarget, upload] = allocation_pair.value();

            m_device->CreateRenderTargetView(custom_rendertarget->GetResource(), nullptr, rtv_cpu_handle);
            custom_rt_srv[i] = m_descriptor_heap.CreateShaderResourceView(m_device.Get(), PALADIN_HASH("BINDLESS"), custom_rendertarget->GetResource(), nullptr);
            rtv_cpu_handle.ptr += rtv_descriptor_size;

            upload_allocations.push_back(std::move(upload));
            auto custom_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(custom_rendertarget->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
            m_command_list->ResourceBarrier(1, &custom_transition_barrier);
            custom_targets.push_back(std::move(custom_rendertarget));
        }
        e+=1;
    }

    PALADIN_LOG(INFO, "E:"+std::to_string(e));

    if (auto hr = m_device->CreateGraphicsPipelineState(&test_state->gpu_pso,IID_PPV_ARGS(&m_pipeline_state)); SUCCEEDED(hr))
    {
        PALADIN_LOG(INFO, "Successfully created TEST pipeline state")
    }
    else if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create Test pipeline state.", hr))
    }
    /*
    aabb_vs = DXShader(VertexShader, L"frustum_vs.hlsl", L"main");
    m_shader_compiler.LoadShader(aabb_vs);
    aabb_fs = DXShader(FragmentShader, L"frustum_fs.hlsl", L"main");
    m_shader_compiler.LoadShader(aabb_fs);

    aabb_state = std::make_unique<PipelineState>(aabb_vs,aabb_fs,m_bindless_root_signature.Get());
    aabb_state->gpu_pso.InputLayout.pInputElementDescs = nullptr;
    aabb_state->gpu_pso.InputLayout.NumElements = 0;
    aabb_state->gpu_pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    aabb_state->gpu_pso.RasterizerState.DepthClipEnable = FALSE;
    aabb_state->gpu_pso.DepthStencilState.DepthEnable = FALSE;

    aabb_state->gpu_pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    if (auto hr = m_device->CreateGraphicsPipelineState(&aabb_state->gpu_pso,IID_PPV_ARGS(&aabb_pipeline)); SUCCEEDED(hr))
    {
        PALADIN_LOG(INFO, "Successfully created AABB pipeline state")
    }
    else if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create AABB pipeline state.", hr))
    }*/
    m_command_list->Close();

    ID3D12CommandList* cmd_lists[] = { m_command_list.Get()};
    m_command_queue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);
    fence_value[frame_index]++;
    auto hr =  m_command_queue->Signal(m_fence[frame_index].Get(), fence_value[frame_index]);

    if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to signal fence.", hr))
    }
   // vertex_buffer_view.BufferLocation = triangle_vertex_buffer->GetGPUVirtualAddress();
   // vertex_buffer_view.SizeInBytes = vertex_buffer_size;
   // vertex_buffer_view.StrideInBytes = sizeof(Vertex);


    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = static_cast<float>(window_width);
    m_viewport.Height = static_cast<float>(window_height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    m_scissor.left = 0;
    m_scissor.top = 0;
    m_scissor.right = static_cast<LONG>(window_width);
    m_scissor.bottom = static_cast<LONG>(window_height);

    ImGui_ImplDX12_InitInfo imgui_init_info = {};
    imgui_init_info.Device = m_device.Get();
    imgui_init_info.CommandQueue = m_command_queue.Get();
    imgui_init_info.NumFramesInFlight = FRAME_BUFFER_COUNT;
    imgui_init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    imgui_init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

    imgui_init_info.UserData = this;

    imgui_init_info.SrvDescriptorHeap = m_imgui_srv_descriptor_heap.Get();
    imgui_init_info.SrvDescriptorAllocFn =  [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
    {
        auto* context = static_cast<D3D12Context*>(info->UserData);
        UINT index = context->imgui_descriptor_index++;

        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = context->m_imgui_srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle = context->m_imgui_srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart();

        cpu_handle.ptr += index * context->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        gpu_handle.ptr += index * context->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        *out_cpu_handle =cpu_handle;
        *out_gpu_handle =gpu_handle;
        return;
    };

    imgui_init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_handle)
    {
        return;
    };

    if (ImGui_ImplDX12_Init(&imgui_init_info))
    {
        PALADIN_LOG(INFO, "IMGUI INITIALIZED FOR DX12")
    }


#ifdef TRACY_ENABLE
    m_tracy_context = TracyD3D12Context(m_device.Get(),m_command_queue.Get())
    if (m_tracy_context == nullptr) {
        PALADIN_LOG(ERR, "Unable to create tracy context.")
    }
    else {
        PALADIN_LOG(INFO,"Tracy context created.");
    }
#endif

}

bool D3D12Context::Render()
{
    PALADIN_SCOPED_CPU_PROFILE("D3D12Context::Render", ProfileColors::Red);
    PALADIN_BEGIN_GPU_PROFILE(m_command_queue.Get(), ProfileColors::Green, "D3D12Context::Render")
    if (m_resized)  Resize(m_window_width,m_window_height);
    TracyD3D12Collect(m_tracy_context)
    TracyD3D12NewFrame(m_tracy_context);
/*
    {
        PALADIN_SCOPED_CPU_PROFILE("VS Hot Reload", ProfileColors::Blue);
        if (test_vs.NeedsRecompilation())[[unlikely]] {
            if (m_shader_compiler.LoadShader(test_vs)) {
                PALADIN_LOG(INFO, "Recompiling "+ConvertWString(test_vs.shader_input_file))
                DXGI_SAMPLE_DESC sample_desc{};
                sample_desc.Count = 1;
                sample_desc.Quality=0;

                D3D12_INPUT_ELEMENT_DESC test_input_layout[] = {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},


                    //{"TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                    //{"TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                    //{"TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                    //{"TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
                };



                D3D12_INPUT_LAYOUT_DESC input_layout_desc ={};

                input_layout_desc.NumElements = sizeof(test_input_layout)/sizeof(D3D12_INPUT_ELEMENT_DESC);
                input_layout_desc.pInputElementDescs = test_input_layout;

                D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};

                pso_desc.InputLayout = input_layout_desc;
                pso_desc.pRootSignature = m_root_signature.Get();
                pso_desc.VS = test_vs.GetBytecode();
                pso_desc.PS = fragment_shader.GetBytecode();
                pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
                pso_desc.SampleDesc = sample_desc;
                pso_desc.SampleMask = 0xfffffff;
                pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC2(D3D12_DEFAULT);
                pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
                pso_desc.NumRenderTargets = 1;
                pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
                pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC2(D3D12_DEFAULT);
                pso_desc.DepthStencilState.DepthEnable = true;

                if (auto hr = m_device->CreateGraphicsPipelineState(&pso_desc,IID_PPV_ARGS(&m_pipeline_state)); SUCCEEDED(hr))
                {
                    PALADIN_LOG(INFO, "Successfully created pipeline state")
                }
                else if (FAILED(hr))
                {
                    PALADIN_LOG(ERR, ErrorResult("Failed to create pipeline state.", hr))
                }

            }
            else {
                PALADIN_LOG(ERR, "Unable to compile vertex shader")
            }
        }
    }*/

    if (!UpdatePipeline()) return false;

    {
        PALADIN_SCOPED_CPU_PROFILE("Execute Command Lists", ProfileColors::Red);
        ID3D12CommandList* command_lists[] = {m_command_list.Get()};
        m_command_queue->ExecuteCommandLists(_countof(command_lists),command_lists);
    }

    {
        PALADIN_SCOPED_CPU_PROFILE("Signal and present" ,ProfileColors::Green);
        fence_value[frame_index]++;
        if (auto hr = m_command_queue->Signal(m_fence[frame_index].Get(),fence_value[frame_index]); FAILED(hr))
        {
            PALADIN_LOG(ERR, ErrorResult("Failed to signal fence post UpdatePipeline", hr))
            return false;
        }

        if (auto hr = m_swap_chain->Present(0,DXGI_PRESENT_ALLOW_TEARING); FAILED(hr))
        {
            PALADIN_LOG(ERR, ErrorResult("Failed to present swap chain", hr))
            return false;
        }
    }
    return true;
}

std::optional<std::pair<Microsoft::WRL::ComPtr<D3D12MA::Allocation>, Microsoft::WRL::ComPtr<D3D12MA::Allocation>>> D3D12Context::CreateAllocation(const D3D12_RESOURCE_DESC& resource_desc, void* data)
{

    //auto info = m_device->GetResourceAllocationInfo(0,1,&resource_desc);


    std::size_t num_bytes = 0;
    UINT row = 0;
    UINT64 row_size = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    switch (resource_desc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        {

            m_device->GetCopyableFootprints(&resource_desc, 0, 1, 0, &footprint, &row, &row_size, &num_bytes);
            break;
        }
    case D3D12_RESOURCE_DIMENSION_BUFFER: num_bytes = resource_desc.Width; break;
    case D3D12_RESOURCE_DIMENSION_UNKNOWN:
        PALADIN_LOG(WARN, "UNKNOWN RESOURCE DIMENSION CALC")
        break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        break;
    }

    //PALADIN_LOG(INFO, "GPU Size Bytes: "+ std::to_string(info.SizeInBytes))
    //PALADIN_LOG(INFO, "CPU Size Bytes: "+ std::to_string(num_bytes))

    D3D12MA::ALLOCATION_DESC allocation_desc = {};
    allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> res = nullptr;
    HRESULT hr;
    if (resource_desc.Flags == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
        D3D12_CLEAR_VALUE optimized_clear_value = {};
        optimized_clear_value.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        //optimized_clear_value.Color = {0.1,0.1,0.1,1.0};

        hr = m_gpu_allocator->CreateResource(&allocation_desc,
        &resource_desc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
    nullptr,
        &allocation,
IID_NULL,
nullptr);
        return std::make_pair(std::move(allocation),nullptr);
    }


    hr = m_gpu_allocator->CreateResource(&allocation_desc,
        &resource_desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        &allocation,
        IID_NULL,
        nullptr);




    if (FAILED(hr))
    {
        WCHAR* stats=nullptr;
        m_gpu_allocator->BuildStatsString(&stats,false);
        std::wstring stat_string(stats);

        PALADIN_LOG(ERR, ConvertWString(stat_string))
        m_gpu_allocator->FreeStatsString(stats);

        PALADIN_LOG(ERR, ErrorResult("Failed to create allocation", hr))


        return std::nullopt;
    }
    else
    {
       // PALADIN_LOG(INFO, "Alloc success");
    }

    D3D12MA::ALLOCATION_DESC upload_allocation_desc = {};
    upload_allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> upload_allocation = nullptr;

    if (resource_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        D3D12_RESOURCE_DESC upload_desc = {};
        upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        upload_desc.Width =  footprint.Footprint.RowPitch*resource_desc.Height;
        upload_desc.Height = 1;
        upload_desc.DepthOrArraySize = 1;
        upload_desc.MipLevels = 1;
        upload_desc.Format = DXGI_FORMAT_UNKNOWN;
        upload_desc.SampleDesc.Count = 1;
        upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        hr = m_gpu_allocator->CreateResource(&upload_allocation_desc,
                                             &upload_desc,
                                             D3D12_RESOURCE_STATE_GENERIC_READ,
                                             nullptr,
                                             &upload_allocation,
                                             IID_NULL, nullptr);
    }
    else
    {
        hr = m_gpu_allocator->CreateResource(&upload_allocation_desc,
            &resource_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            &upload_allocation,
            IID_NULL,
            nullptr);
    }


    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create upload allocation", hr))
        return std::nullopt;
    }
    else
    {
        //PALADIN_LOG(INFO, "Upload Alloc creation success");
    }

    if (resource_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        void* destination = nullptr;

        hr = upload_allocation->GetResource()->Map(0,nullptr,&destination);
        if (FAILED(hr))
        {
            PALADIN_LOG(ERR, ErrorResult("Failed to map upload allocation", hr))
        }

        UINT64 bytes_per_row = resource_desc.Width*4;
        for (UINT current_row=0; current_row < resource_desc.Height; current_row++)
        {
            BYTE* src_row_ptr = static_cast<BYTE*>(data);
            BYTE* dest_row_ptr = static_cast<BYTE*>(destination) + (current_row * footprint.Footprint.RowPitch);
            std::memcpy(dest_row_ptr, src_row_ptr+(current_row*bytes_per_row), bytes_per_row);

        }


        upload_allocation->GetResource()->Unmap(0,nullptr);
        return std::make_pair(std::move(allocation), std::move(upload_allocation));
    }

    void* destination = nullptr;
    hr = upload_allocation->GetResource()->Map(0,nullptr,&destination);
    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to map upload allocation", hr))
    }
    std::memcpy(destination, data, num_bytes);
    upload_allocation->GetResource()->Unmap(0,nullptr);

    return std::make_pair(std::move(allocation), std::move(upload_allocation));
}

void D3D12Context::CreateVisibleInstanceIDBuffer() {

    std::size_t bytes_ = sizeof(std::uint32_t)* MAX_INSTANCES;
    temp_aligned_byes = (bytes_ + 255) & ~255;
    UINT aligned_size =temp_aligned_byes * FRAME_BUFFER_COUNT;



    auto _buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(aligned_size);

    HRESULT hr;
    D3D12MA::ALLOCATION_DESC upload_allocation_desc = {};
    upload_allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

    hr = m_gpu_allocator->CreateResource(&upload_allocation_desc,
   &_buffer_desc,
   D3D12_RESOURCE_STATE_GENERIC_READ,
   nullptr,
   &m_instance_id_data,
   IID_NULL, nullptr);

    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create upload allocation", hr))
        //return nullptr;
    }

    CD3DX12_RANGE read_range(0, 0);
    hr = m_instance_id_data->GetResource()->Map(0,&read_range,&id_data_destination);
    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to map upload allocation", hr))
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Buffer.NumElements = aligned_size/4;
    srv_desc.Buffer.StructureByteStride = 0;
    srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

    instance_id_view = m_descriptor_heap.CreateShaderResourceView(m_device.Get(), PALADIN_HASH("BINDLESS"), m_instance_id_data->GetResource(), &srv_desc);

    instance_draw_id_entry.aligned_size_bytes = temp_aligned_byes;
    instance_draw_id_entry.heap_identifier = instance_id_view.index;
}




void D3D12Context::CreateInstanceBuffer() {
    std::size_t bytes_per_buffer = sizeof(TransformData)* MAX_INSTANCES;
    instance_buffer_entry.aligned_size_bytes = (bytes_per_buffer + 255) & ~255;
    UINT aligned_size_total =(instance_buffer_entry.aligned_size_bytes * FRAME_BUFFER_COUNT)+ 255 & ~255;

    auto instance_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(aligned_size_total);

    HRESULT hr;
    D3D12MA::ALLOCATION_DESC upload_allocation_desc = {};
    upload_allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

    hr = m_gpu_allocator->CreateResource(&upload_allocation_desc,
   &instance_buffer_desc,
   D3D12_RESOURCE_STATE_GENERIC_READ,
   nullptr,
   &m_instance_data,
   IID_NULL, nullptr);

    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create upload allocation", hr))
        //return nullptr;
    }

    CD3DX12_RANGE read_range(0, 0);
    hr = m_instance_data->GetResource()->Map(0,&read_range,&instance_data_destination);
    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to map upload allocation", hr))
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Buffer.NumElements = aligned_size_total/4;
    srv_desc.Buffer.StructureByteStride = 0;
    srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

    instance_buffer_view = m_descriptor_heap.CreateShaderResourceView(m_device.Get(), PALADIN_HASH("BINDLESS"), m_instance_data->GetResource(), &srv_desc);

    instance_buffer_entry.heap_identifier= instance_buffer_view.index;
}


void D3D12Context::UpdateInstanceBufferT(std::span<TransformData> instance_data) {
   /* instance_count = instance_data.size();
    std::size_t total_bytes = sizeof(TransformData)*instance_data.size();

    UINT8* destination = static_cast<UINT8*>(instance_data_destination) + (frame_index * aligned_bytes_per_buffer);
    std::memcpy(destination, instance_data.data(), total_bytes);*/
}


void D3D12Context::UpdateFrameData(FrameUploadData &render_frame_data) {
    //TODO: REFACTOR AT SOME POINT
    render_frame_data.aligned_bytes_per_buffer = instance_buffer_entry.aligned_size_bytes;
    render_frame_data.frame_index = frame_index;
    render_frame_data.instance_buffer_id = instance_buffer_view.index;
    render_frame_data.id_aligned_bytes = temp_aligned_byes;
    render_frame_data.id_heap_index = instance_id_view.index;
    temp_frame_data = render_frame_data;

    UINT8* destination = static_cast<UINT8*>(instance_data_destination) + (frame_index * instance_buffer_entry.aligned_size_bytes);
    std::memcpy(destination, render_frame_data.transforms.data(), sizeof(TransformData)*render_frame_data.transforms.size());

    UINT8* id_destination = static_cast<UINT8*>(id_data_destination) + (frame_index * instance_draw_id_entry.aligned_size_bytes);
    std::memcpy(id_destination, render_frame_data.instance_indices.data(), sizeof(std::uint32_t)*render_frame_data.instance_indices.size());
}


void D3D12Context::UpdateRenderFrameData(RenderFrameData &render_frame_data) {
/*
    render_frame_data.aligned_bytes_per_buffer = aligned_bytes_per_buffer;
    render_frame_data.frame_index = frame_index;
    render_frame_data.instance_buffer_id = instance_buffer_view.index;

    _frame_data = render_frame_data;
    UINT8* destination = static_cast<UINT8*>(instance_data_destination) + (frame_index * aligned_bytes_per_buffer);
    std::size_t byte_offset = 0;
    for (auto& batch : render_frame_data.batches) {
        const std::size_t total_bytes = sizeof(TransformData)*batch.transforms.size();
        std::memcpy(destination +byte_offset, batch.transforms.data(), total_bytes);
        byte_offset+=total_bytes;
    }
    auto aabb_count = _frame_data.debug_aabb_transforms.size();
    std::size_t aabb_bytes = sizeof(TransformData)*aabb_count;
    std::memcpy(destination + byte_offset, _frame_data.debug_aabb_transforms.data(), aabb_bytes);*/
}


MaterialIndices D3D12Context::GetMaterialIndices(OpaqueAssetHandle material_handle)
{

    auto material = m_gpu_resources.GetOpaque<GPUMaterial>(material_handle);

    std::uint32_t albedo_srv_index = 0;
    if ( auto albedo = m_gpu_resources.Get<GPUTexture2D>(material->albedo); albedo != nullptr) {
        albedo_srv_index = albedo->srv_handle.index;
    }

    std::uint32_t normal_srv_index = 0;
    if (auto normal = m_gpu_resources.Get<GPUTexture2D>(material->normal);normal != nullptr) {
        normal_srv_index = normal->srv_handle.index;
    }


    std::uint32_t roughness_srv_handle = 0;
    if (auto roughness = m_gpu_resources.Get<GPUTexture2D>(material->roughness);roughness != nullptr) {
        roughness_srv_handle = roughness->srv_handle.index;
    }

    std::uint32_t metal_srv_handle = 0;
    if (auto metallic = m_gpu_resources.Get<GPUTexture2D>(material->metallic);metallic != nullptr) {
        metal_srv_handle = metallic->srv_handle.index;
    }



    auto mat_indices = MaterialIndices{
        .albedo = albedo_srv_index,
        .normal = normal_srv_index,
        .roughness = roughness_srv_handle,
        .metallic = metal_srv_handle
    };


    return mat_indices;
}

void D3D12Context::CreatePersistantAllocation(CameraUniform uniform_data) {

    std::size_t camera_bytes = sizeof(CameraUniform);
    UINT aligned_size = (camera_bytes + 255) & ~255;

    auto camera_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(aligned_size*FRAME_BUFFER_COUNT);

    HRESULT hr;
    D3D12MA::ALLOCATION_DESC upload_allocation_desc = {};
    upload_allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

    hr = m_gpu_allocator->CreateResource(&upload_allocation_desc,
   &camera_buffer_desc,
   D3D12_RESOURCE_STATE_GENERIC_READ,
   nullptr,
   &m_frame_data,
   IID_NULL, nullptr);

    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create upload allocation", hr))
        //return nullptr;
    }

    CD3DX12_RANGE read_range(0, 0);
    hr = m_frame_data->GetResource()->Map(0,&read_range,&frame_data_destination);
    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to map upload allocation", hr))
    }
    for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
        void* dest = static_cast<BYTE*>(frame_data_destination) + (i * aligned_size);
        std::memcpy(dest , &uniform_data, sizeof(CameraUniform));
    }
}

void D3D12Context::UpdatePersistantAllocation(CameraUniform uniform_data) {
    std::size_t camera_bytes = sizeof(CameraUniform);
    const UINT aligned_size = (camera_bytes + 255) & ~255;

    UINT8* destination = static_cast<UINT8*>(frame_data_destination) + (frame_index * aligned_size);
    std::memcpy(destination, &uniform_data, sizeof(CameraUniform));
}

GPUResourceHandle<GPUTexture2D> D3D12Context::UploadTexture2D(Texture2DAsset& texture, OpaqueAssetHandle handle)
{

    if (m_gpu_resources.IsValid<GPUTexture2D>(handle)) {
        if (m_gpu_resources.GetOpaque<GPUTexture2D>(handle) != nullptr) {
            return GPUResourceHandle<GPUTexture2D>{handle.inner};
        }
    }

    PALADIN_SCOPED_CPU_PROFILE("Upload Texture", ProfileColors::Red);

    WaitForPreviousFrame();
    HRESULT hr;
    m_command_list->Reset(m_command_allocator[frame_index].Get(), nullptr);
    std::size_t num_bytes = 0;
    UINT row = 0;
    UINT64 row_size = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;


    auto gpu_texture = std::make_unique<GPUTexture2D>();

    auto texture_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
    texture.width,
    texture.height,
    1,
    1,
    1,
    0,
    D3D12_RESOURCE_FLAG_NONE,
    D3D12_TEXTURE_LAYOUT_UNKNOWN,
    0);
    m_device->GetCopyableFootprints(&texture_desc, 0, 1, 0, &footprint, &row, &row_size, &num_bytes);


    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> upload_buffers;

    if (auto allocation_pair = CreateAllocation(texture_desc,texture.pixels.data()); allocation_pair.has_value())
    {
        {
            auto [texture_allocation, texture_upload_allocation] = allocation_pair.value();
            //PALADIN_SCOPED_GPU_PROFILE_C(m_tracy_context, m_command_list.Get(), "Copying Texture Data", ProfileColors::Red)

            D3D12_TEXTURE_COPY_LOCATION dst_loc = {};
            dst_loc.pResource = texture_allocation->GetResource();
            dst_loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst_loc.SubresourceIndex = 0;

            D3D12_TEXTURE_COPY_LOCATION src_loc = {};
            src_loc.pResource = texture_upload_allocation->GetResource();
            src_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src_loc.PlacedFootprint = footprint;
            src_loc.PlacedFootprint.Offset=0;
            m_command_list->CopyTextureRegion(&dst_loc,0,0,0,&src_loc,nullptr);




            auto transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture_allocation->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            upload_buffers.push_back(std::move(texture_upload_allocation));
            gpu_texture->allocation = std::move(texture_allocation);
            m_command_list->ResourceBarrier(1, &transition_barrier);
        }
    }
    else {
        return {};
    }

    hr = m_command_list->Close();
    if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to close command list", hr));
    }
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    fence_value[frame_index]++;
    hr =  m_command_queue->Signal(m_fence[frame_index].Get(), fence_value[frame_index]);

    if (m_fence[frame_index]->GetCompletedValue() < fence_value[frame_index]) {
        m_fence[frame_index]->SetEventOnCompletion(fence_value[frame_index], fence_event);
        WaitForSingleObject(fence_event, INFINITE);
    }




    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.Texture2D.MipLevels = 1;

    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    gpu_texture->srv_handle = m_descriptor_heap.CreateShaderResourceView(
        m_device.Get(),
        PALADIN_HASH("BINDLESS"),
        gpu_texture->allocation->GetResource(),
        &srv_desc);

    return m_gpu_resources.Insert(std::move(gpu_texture), handle);
}

GPUResourceHandle<GPUMesh> D3D12Context::UploadMesh(MeshAsset& mesh, OpaqueAssetHandle handle)
{
    PALADIN_SCOPED_CPU_PROFILE("Upload Mesh", ProfileColors::Red);

    WaitForPreviousFrame();
    HRESULT hr;
    m_command_list->Reset(m_command_allocator[frame_index].Get(), nullptr);

    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> upload_buffers;

    std::size_t vertex_total_bytes = mesh.vertices.size()*sizeof(Paladin::Vertex);
    std::size_t index_total_bytes = mesh.indices.size()*sizeof(std::uint32_t);

    auto vertex_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_total_bytes);
    auto index_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(index_total_bytes);

    auto gpu_mesh = std::make_unique<GPUMesh>();
    gpu_mesh->indices = mesh.indices.size();
    gpu_mesh->material = mesh.material;
    gpu_mesh->aabb = mesh.bounding_box;

    if (auto allocation_pair = CreateAllocation(vertex_buffer_desc,(void*)mesh.vertices.data()); allocation_pair.has_value())
    {
        {
            auto [vertex_allocation, vertex_upload_allocation] = allocation_pair.value();
            m_command_list->CopyBufferRegion(vertex_allocation->GetResource(),0,vertex_upload_allocation->GetResource(),0,vertex_total_bytes);
            D3D12_VERTEX_BUFFER_VIEW vertex_view = {};
            vertex_view.BufferLocation = vertex_allocation->GetResource()->GetGPUVirtualAddress();
            vertex_view.SizeInBytes = vertex_total_bytes;
            vertex_view.StrideInBytes = sizeof(Paladin::Vertex);


            auto vertex_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_allocation->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

            upload_buffers.push_back(std::move(vertex_upload_allocation));
            gpu_mesh->vertex_allocation = std::move(vertex_allocation);
            gpu_mesh->vertex_buffer_view = vertex_view;

            m_command_list->ResourceBarrier(1, &vertex_transition_barrier);
        }



    }

    if (auto allocation_pair = CreateAllocation(index_buffer_desc,(void*)mesh.indices.data()); allocation_pair.has_value())
    {
        {
            auto [index_allocation, index_upload_allocation] = allocation_pair.value();
            //PALADIN_SCOPED_GPU_PROFILE_C(m_tracy_context, m_command_list.Get(), "Copying Mesh index Data", ProfileColors::Red)
            m_command_list->CopyBufferRegion(index_allocation->GetResource(),0,index_upload_allocation->GetResource(),0,index_total_bytes);

            D3D12_INDEX_BUFFER_VIEW index_buffer_view = {};
            index_buffer_view.BufferLocation = index_allocation->GetResource()->GetGPUVirtualAddress();
            index_buffer_view.SizeInBytes = index_total_bytes;
            index_buffer_view.Format = DXGI_FORMAT_R32_UINT;

            auto index_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(index_allocation->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

            upload_buffers.push_back(std::move(index_upload_allocation));
            gpu_mesh->index_allocation = std::move(index_allocation);
            gpu_mesh->index_buffer_view = index_buffer_view;
            //index_buffers.emplace_back(index_buffer_view,std::move(index_allocation));
            m_command_list->ResourceBarrier(1, &index_transition_barrier);
        }
    }


    hr = m_command_list->Close();
    if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to close command list", hr));
    }
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    fence_value[frame_index]++;
    hr =  m_command_queue->Signal(m_fence[frame_index].Get(), fence_value[frame_index]);

    if (m_fence[frame_index]->GetCompletedValue() < fence_value[frame_index]) {
        m_fence[frame_index]->SetEventOnCompletion(fence_value[frame_index], fence_event);
        WaitForSingleObject(fence_event, INFINITE);
    }

    return m_gpu_resources.Insert<GPUMesh>(std::move(gpu_mesh), handle);
   // m_gpu_resources.Insert<GPUMesh>()

}


void D3D12Context::UploadMeshDescriptors(std::vector<MeshDescriptor> mesh_descriptors, std::map<OpaqueAssetHandle, std::uint32_t> descriptor_fetch)
{
    PALADIN_SCOPED_CPU_PROFILE("Upload Mesh Descriptors", ProfileColors::Red);
    m_mesh_descriptors = mesh_descriptors;
    m_descriptor_fetch = descriptor_fetch;


    WaitForPreviousFrame();
    HRESULT hr;
    m_command_list->Reset(m_command_allocator[frame_index].Get(), nullptr);

    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> upload_buffers;

    std::size_t buff_total_bytes = mesh_descriptors.size()*sizeof(MeshDescriptor);
    auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(buff_total_bytes);


    if (auto allocation_pair = CreateAllocation(buffer_desc,(void*)mesh_descriptors.data()); allocation_pair.has_value())
    {
        {
            auto [buffer_allocation, buffer_upload_allocation] = allocation_pair.value();
            m_command_list->CopyBufferRegion(buffer_allocation->GetResource(),0,buffer_upload_allocation->GetResource(),0,buff_total_bytes);

            auto buffer_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer_allocation->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Buffer.NumElements = buff_total_bytes/4;
            srv_desc.Buffer.StructureByteStride = 0;
            srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

            upload_buffers.push_back(std::move(buffer_upload_allocation));
            m_mesh_descriptor_allocation = std::move(buffer_allocation);
            m_mesh_descriptor_view = m_descriptor_heap.CreateShaderResourceView(m_device.Get(),PALADIN_HASH("BINDLESS"),m_mesh_descriptor_allocation->GetResource(), &srv_desc);


            m_command_list->ResourceBarrier(1, &buffer_transition_barrier);
        }
    }
    hr = m_command_list->Close();
    if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to close command list", hr));
    }
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    fence_value[frame_index]++;
    hr =  m_command_queue->Signal(m_fence[frame_index].Get(), fence_value[frame_index]);

    if (m_fence[frame_index]->GetCompletedValue() < fence_value[frame_index]) {
        m_fence[frame_index]->SetEventOnCompletion(fence_value[frame_index], fence_event);
        WaitForSingleObject(fence_event, INFINITE);
    }
}


void D3D12Context::UploadIVBuffers(std::span<Paladin::Vertex> vertices, std::span<std::uint32_t> indices)
{
    PALADIN_SCOPED_CPU_PROFILE("Upload Main V Buffer and I Buffer", ProfileColors::Red);

    WaitForPreviousFrame();
    HRESULT hr;
    m_command_list->Reset(m_command_allocator[frame_index].Get(), nullptr);

    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> upload_buffers;

    std::size_t vertex_total_bytes = vertices.size()*sizeof(Paladin::Vertex);
    //std::uint32_t aligned_vertex_size =     (vertex_total_bytes + 255) & ~255;
    std::size_t index_total_bytes = indices.size()*sizeof(std::uint32_t);
   // std::uint32_t aligned_indices_size =     (index_total_bytes + 255) & ~255;

    auto vertex_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_total_bytes);
    auto index_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(index_total_bytes);


    if (auto allocation_pair = CreateAllocation(vertex_buffer_desc,(void*)vertices.data()); allocation_pair.has_value())
    {
        {
            auto [vertex_allocation, vertex_upload_allocation] = allocation_pair.value();
            m_command_list->CopyBufferRegion(vertex_allocation->GetResource(),0,vertex_upload_allocation->GetResource(),0,vertex_total_bytes);

            auto vertex_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_allocation->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Buffer.NumElements = vertex_total_bytes/4;
            srv_desc.Buffer.StructureByteStride = 0;
            srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

            upload_buffers.push_back(std::move(vertex_upload_allocation));
            m_vertex_buffer = std::move(vertex_allocation);
            m_vertex_buffer_view = m_descriptor_heap.CreateShaderResourceView(m_device.Get(),PALADIN_HASH("BINDLESS"),m_vertex_buffer->GetResource(), &srv_desc);


            m_command_list->ResourceBarrier(1, &vertex_transition_barrier);
        }



    }

    if (auto allocation_pair = CreateAllocation(index_buffer_desc,(void*)indices.data()); allocation_pair.has_value())
    {
        {
            auto [index_allocation, index_upload_allocation] = allocation_pair.value();
            //PALADIN_SCOPED_GPU_PROFILE_C(m_tracy_context, m_command_list.Get(), "Copying Mesh index Data", ProfileColors::Red)
            m_command_list->CopyBufferRegion(index_allocation->GetResource(),0,index_upload_allocation->GetResource(),0,index_total_bytes);

            auto index_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(index_allocation->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);


            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Buffer.NumElements = index_total_bytes/4;
            srv_desc.Buffer.StructureByteStride = 0;
            srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

            upload_buffers.push_back(std::move(index_upload_allocation));
            m_index_buffer = std::move(index_allocation);
            m_index_buffer_view = m_descriptor_heap.CreateShaderResourceView(m_device.Get(),PALADIN_HASH("BINDLESS"),m_index_buffer->GetResource(), &srv_desc);
            m_command_list->ResourceBarrier(1, &index_transition_barrier);
        }
    }


    hr = m_command_list->Close();
    if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to close command list", hr));
    }
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    fence_value[frame_index]++;
    hr =  m_command_queue->Signal(m_fence[frame_index].Get(), fence_value[frame_index]);

    if (m_fence[frame_index]->GetCompletedValue() < fence_value[frame_index]) {
        m_fence[frame_index]->SetEventOnCompletion(fence_value[frame_index], fence_event);
        WaitForSingleObject(fence_event, INFINITE);
    }
}

GPUResourceHandle<GPUMaterial> D3D12Context::AddMaterial(std::unique_ptr<GPUMaterial> material, OpaqueAssetHandle handle) {
    return m_gpu_resources.Insert<GPUMaterial>(std::move(material), handle);
}

GPUResourceHandle<GPUModel> D3D12Context::AddModel(std::unique_ptr<GPUModel> model, OpaqueAssetHandle handle) {
    model_handle = handle;
    return m_gpu_resources.Insert<GPUModel>(std::move(model), handle);
}

void D3D12Context::UploadModel(std::span<Paladin::Vertex> model_vertices, std::span<std::uint32_t> model_indices,  std::vector<MeshDescriptor> mesh_ranges) {
    PALADIN_LOG(INFO, "Uploading model")
    PALADIN_SCOPED_CPU_PROFILE("Upload Model", ProfileColors::Red);

    WaitForPreviousFrame();
    HRESULT hr;
    m_command_list->Reset(m_command_allocator[frame_index].Get(), nullptr);
    //PALADIN_SCOPED_GPU_PROFILE_C(m_tracy_context, m_command_list.Get(), "Loading Model", ProfileColors::Blue)
    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> upload_buffers;

    std::size_t vertex_total_bytes = model_vertices.size()*sizeof(Paladin::Vertex);
    std::size_t index_total_bytes = model_indices.size()*sizeof(std::uint32_t);

    auto vertex_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_total_bytes);
    auto index_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(index_total_bytes);

    if (auto allocation_pair = CreateAllocation(vertex_buffer_desc,model_vertices.data()); allocation_pair.has_value())
    {
        {
            auto [vertex_allocation, vertex_upload_allocation] = allocation_pair.value();
            //PALADIN_SCOPED_GPU_PROFILE_C(m_tracy_context, m_command_list.Get(), "Copying Model Data", ProfileColors::Red)
            m_command_list->CopyBufferRegion(vertex_allocation->GetResource(),0,vertex_upload_allocation->GetResource(),0,vertex_total_bytes);

            D3D12_VERTEX_BUFFER_VIEW vertex_view = {};
            vertex_view.BufferLocation = vertex_allocation->GetResource()->GetGPUVirtualAddress();
            vertex_view.SizeInBytes = vertex_total_bytes;
            vertex_view.StrideInBytes = sizeof(Paladin::Vertex);


            auto vertex_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertex_allocation->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

            upload_buffers.push_back(std::move(vertex_upload_allocation));
            vertex_buffers.emplace_back(vertex_view,std::move(vertex_allocation));

            m_command_list->ResourceBarrier(1, &vertex_transition_barrier);
        }
    }

    if (auto allocation_pair = CreateAllocation(index_buffer_desc,model_indices.data()); allocation_pair.has_value())
    {
        {
            auto [index_allocation, index_upload_allocation] = allocation_pair.value();
            //PALADIN_SCOPED_GPU_PROFILE_C(m_tracy_context, m_command_list.Get(), "Copying Model Data", ProfileColors::Red)
            m_command_list->CopyBufferRegion(index_allocation->GetResource(),0,index_upload_allocation->GetResource(),0,index_total_bytes);

            D3D12_INDEX_BUFFER_VIEW index_buffer_view = {};
            index_buffer_view.BufferLocation = index_allocation->GetResource()->GetGPUVirtualAddress();
            index_buffer_view.SizeInBytes = index_total_bytes;
            index_buffer_view.Format = DXGI_FORMAT_R32_UINT;

            auto index_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(index_allocation->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

            upload_buffers.push_back(std::move(index_upload_allocation));
            index_buffers.emplace_back(index_buffer_view,std::move(index_allocation));

            m_command_list->ResourceBarrier(1, &index_transition_barrier);
        }
    }

    hr = m_command_list->Close();
    if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to close command list", hr));
    }
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    fence_value[frame_index]++;
    hr =  m_command_queue->Signal(m_fence[frame_index].Get(), fence_value[frame_index]);

    if (m_fence[frame_index]->GetCompletedValue() < fence_value[frame_index]) {
        m_fence[frame_index]->SetEventOnCompletion(fence_value[frame_index], fence_event);
        WaitForSingleObject(fence_event, INFINITE);
    }


    model_mesh_ranges.push_back(mesh_ranges);
}


bool D3D12Context::Resize(std::uint32_t new_width, std::uint32_t new_height) {
    PALADIN_SCOPED_CPU_PROFILE("ResizeEvent", ProfileColors::Blue);
    m_resized = false;

    FlushDevice();
    m_command_list->Reset(m_command_allocator[frame_index].Get(), nullptr);


    for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
        m_render_target[i].Reset();
    }
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = static_cast<float>(new_width);
    m_viewport.Height = static_cast<float>(new_height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    m_scissor.left = 0;
    m_scissor.top = 0;
    m_scissor.right = static_cast<LONG>(new_width);
    m_scissor.bottom = static_cast<LONG>(new_height);


    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};

    if (auto hr = m_swap_chain->GetDesc1(&swap_chain_desc); FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to get swap chain desc on resize", hr))
        return false;
    }

    if (auto hr = m_swap_chain->ResizeBuffers(FRAME_BUFFER_COUNT, new_width, new_height, swap_chain_desc.Format, swap_chain_desc.Flags);FAILED(hr)) {
        D3D12DebugLayer::LogDebugMessages(m_device.Get());
        PALADIN_LOG(ERR, ErrorResult("Failed to resize buffers", hr))
        return false;
    }

    auto rtv_descriptor_size =  m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle(m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        if (auto hr = m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_render_target[i])); SUCCEEDED(hr)) {
            //PALADIN_LOG(INFO, "Created render target")
        }
        else if (FAILED(hr)) {
            PALADIN_LOG(ERR, ErrorResult("Failed to create render target", hr))
        }
        m_device->CreateRenderTargetView(m_render_target[i].Get(), nullptr, rtv_cpu_handle);
        rtv_cpu_handle.ptr += rtv_descriptor_size;
    }

    int channels = 4;
    std::vector<float> blank_data;
    blank_data.resize(new_width*new_height*channels);


    //auto srv_handle  = m_srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
    auto descriptor_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto viewport_srv_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32G32B32A32_FLOAT,
        new_width,
        new_height,
        1,
        1,
        1,
        0,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        0);

    std::vector<Microsoft::WRL::ComPtr<D3D12MA::Allocation>> upload_allocations;
  //  srv_handle.ptr += custom_render_target_index*descriptor_size;
    rtv_cpu_handle = m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
    rtv_cpu_handle.ptr += custom_rtv_target_index * rtv_descriptor_size;
    for (UINT i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        if (auto allocation_pair = CreateAllocation(viewport_srv_desc,blank_data.data());allocation_pair.has_value()) {
            custom_targets[i].Reset();
            auto [custom_rendertarget, upload] = allocation_pair.value();

            m_device->CreateRenderTargetView(custom_rendertarget->GetResource(), nullptr, rtv_cpu_handle);
            m_device->CreateShaderResourceView(custom_rendertarget->GetResource(), nullptr, m_descriptor_heap.GetCPUHandle(custom_rt_srv[i]));
            upload_allocations.push_back(std::move(upload));
            auto custom_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(custom_rendertarget->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
            rtv_cpu_handle.ptr += rtv_descriptor_size;

            m_command_list->ResourceBarrier(1, &custom_transition_barrier);
            custom_targets[i] = std::move(custom_rendertarget);
        }
    }



    D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
    depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depth_stencil_view_desc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_RESOURCE_DESC depth_stencil_desc = {};
    depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depth_stencil_desc.Width = new_width;
    depth_stencil_desc.Height = new_height;
    depth_stencil_desc.MipLevels = 1;
    depth_stencil_desc.DepthOrArraySize = 1;
    depth_stencil_desc.SampleDesc.Count = 1;
    depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    depth_stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;

    D3D12_CLEAR_VALUE depth_optimized_clear_value = {};

    depth_optimized_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    depth_optimized_clear_value.DepthStencil.Depth = 1.0f;
    depth_optimized_clear_value.DepthStencil.Stencil = 0;

    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        m_depth_buffer[i].Reset();
    }

    D3D12MA::ALLOCATION_DESC allocation_desc = {};
    allocation_desc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {

        m_gpu_allocator->CreateResource(&allocation_desc,
            &depth_stencil_desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depth_optimized_clear_value,
            &m_depth_buffer[i],
            IID_NULL,
            nullptr);
        auto descriptor_dest = m_descriptor_heap.GetCPUHandle(depth_views[i]);
        m_device->CreateDepthStencilView(m_depth_buffer[i]->GetResource(), &depth_stencil_view_desc, descriptor_dest);
    }

    auto hr = m_command_list->Close();
    if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to close command list", hr));
    }
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    fence_value[frame_index]++;
    hr =  m_command_queue->Signal(m_fence[frame_index].Get(), fence_value[frame_index]);

    if (m_fence[frame_index]->GetCompletedValue() < fence_value[frame_index]) {
        m_fence[frame_index]->SetEventOnCompletion(fence_value[frame_index], fence_event);
        WaitForSingleObject(fence_event, INFINITE);
    }

    frame_index = m_swap_chain->GetCurrentBackBufferIndex();
    return true;
}




bool D3D12Context::WaitForPreviousFrame()
{
    PALADIN_SCOPED_CPU_PROFILE("WaitForPreviousFrame", ProfileColors::Red);
    frame_index = m_swap_chain->GetCurrentBackBufferIndex();

    const UINT64 fence_waiting = fence_value[frame_index];
    if (m_fence[frame_index]->GetCompletedValue()<fence_waiting){
        if (auto hr = m_fence[frame_index]->SetEventOnCompletion(fence_waiting,fence_event); FAILED(hr))
        {
            PALADIN_LOG(ERR, ErrorResult("Failed to set fence event",hr))
            return false;
        }
        WaitForSingleObject(fence_event, INFINITE);
    }

    return true;
}

void D3D12Context::CreateIndirectCommandBuffer() {

    std::size_t bytes_per_buffer = sizeof(InstancedIndirectCmd)* MAX_COMMANDS;
    command_buffer_entry.aligned_size_bytes = (bytes_per_buffer + 255) & ~255;
    UINT aligned_size_total =(command_buffer_entry.aligned_size_bytes * FRAME_BUFFER_COUNT)+ 255 & ~255;

    auto indirect_command_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(aligned_size_total);

    HRESULT hr;
    D3D12MA::ALLOCATION_DESC upload_allocation_desc = {};
    upload_allocation_desc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

    hr = m_gpu_allocator->CreateResource(&upload_allocation_desc,
   &indirect_command_buffer_desc,
   D3D12_RESOURCE_STATE_GENERIC_READ,
   nullptr,
   &m_indirect_command_data,
   IID_NULL, nullptr);

    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create upload allocation", hr))
        //return nullptr;
    }

    CD3DX12_RANGE read_range(0, 0);
    hr = m_indirect_command_data->GetResource()->Map(0,&read_range,&indirect_command_buffer_dest);
    if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to map upload allocation", hr))
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Buffer.NumElements = aligned_size_total/4;
    srv_desc.Buffer.StructureByteStride = 0;
    srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

    command_buffer_view = m_descriptor_heap.CreateShaderResourceView(m_device.Get(), PALADIN_HASH("BINDLESS"), m_indirect_command_data->GetResource(), &srv_desc);
    command_buffer_entry.heap_identifier= command_buffer_view.index;
}

void D3D12Context::UpdateIndirectCommandBuffer(std::span<InstancedIndirectCmd> arguments) {
    UINT8* destination = static_cast<UINT8*>(indirect_command_buffer_dest) + (frame_index * command_buffer_entry.aligned_size_bytes);
    std::memcpy(destination, arguments.data(), sizeof(InstancedIndirectCmd)*arguments.size());
}

bool D3D12Context::UpdatePipeline()
{
    PALADIN_SCOPED_CPU_PROFILE("D3D12Context::UpdatePipeline",ProfileColors::Red);
    if (auto hr = m_command_allocator[frame_index]->Reset(); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to reset command allocator.", hr))
        return false;
    }
    if (auto hr = m_command_list->Reset(m_command_allocator[frame_index].Get(),m_pipeline_state.Get()); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to reset command allocator.", hr))
        return false;
    }

    {
        PALADIN_SCOPED_GPU_PROFILE_C(m_tracy_context, m_command_list.Get(), "Draw Commands", ProfileColors::Green)

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGuiID dockspace_id = ImGui::GetID("EditorWindow");
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            ImGuiID dock_id_left = 0;
            ImGuiID dock_id_main = dockspace_id;
            ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, &dock_id_left, &dock_id_main);
            ImGuiID dock_id_left_top = 0;
            ImGuiID dock_id_left_bottom = 0;
            ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.50f, &dock_id_left_top, &dock_id_left_bottom);
            ImGui::DockBuilderDockWindow("Game", dock_id_main);
            ImGui::DockBuilderDockWindow("Properties", dock_id_left_top);
            ImGui::DockBuilderDockWindow("Scene", dock_id_left_bottom);
            ImGui::DockBuilderFinish(dockspace_id);
        }
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        m_command_list->RSSetViewports(1, &m_viewport);
        m_command_list->RSSetScissorRects(1,&m_scissor);
        const float clear_colour[] = { 0.0f,0.2f,0.4f,1.0f };


        auto custom_rtv_handle = m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        custom_rtv_handle.ptr += (custom_rtv_target_index+frame_index)*m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        m_command_list->ClearRenderTargetView(custom_rtv_handle ,clear_colour,0,nullptr);

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_render_target[frame_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_command_list->ResourceBarrier(1,&barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),frame_index,m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
        auto depth_handle = m_descriptor_heap.GetCPUHandle(depth_views[frame_index]);

        m_command_list->OMSetRenderTargets(1, &custom_rtv_handle, false, &depth_handle);

        m_command_list->ClearRenderTargetView(rtv_handle,clear_colour,0,nullptr);

        m_command_list->ClearDepthStencilView(depth_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0,0,0,nullptr);

        m_command_list->SetPipelineState(m_pipeline_state.Get());
        ID3D12DescriptorHeap* heaps[] = { m_descriptor_heap.GetDescriptorHeap(PALADIN_HASH("BINDLESS"))};
        m_command_list->SetDescriptorHeaps(1,heaps);

        m_command_list->SetGraphicsRootSignature(m_bindless_root_signature.Get());

        D3D12_GPU_VIRTUAL_ADDRESS frame_address = m_frame_data->GetResource()->GetGPUVirtualAddress() + (frame_index * 256);
        m_command_list->SetGraphicsRootConstantBufferView(0,frame_address);

        m_command_list->SetGraphicsRootDescriptorTable(1,m_descriptor_heap.GetStartGPUHandle(PALADIN_HASH("BINDLESS")));

        m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        std::uint32_t current_offset = 0;

        PerFrameData pfd = {
            .frame_index = frame_index,
            .ib_buffer_bytes = instance_buffer_entry.aligned_size_bytes,
            .ib_heap_index = instance_buffer_entry.heap_identifier,
            .draw_id_buffer_bytes = instance_draw_id_entry.aligned_size_bytes,
            .draw_id_heap_index = instance_draw_id_entry.heap_identifier,

            .vertex_heap_index = m_vertex_buffer_view.index,
            .index_heap_index = m_index_buffer_view.index,
            .descriptor_heap_index = m_mesh_descriptor_view.index,
        };

        m_command_list->SetGraphicsRoot32BitConstants(4,8,&pfd,0);


        std::vector<InstancedIndirectCmd> indirect_cmds;
        indirect_cmds.reserve(temp_frame_data.visible_meshes.size());
        for (auto mesh : temp_frame_data.visible_meshes) {
            const auto descriptor_index = m_descriptor_fetch[mesh.mesh_handle];
            const auto& descriptor = m_mesh_descriptors[descriptor_index];
            InstancedIndirectCmd indirect_cmd = {
                .draw_index = descriptor_index,
                .instance_index = mesh.instance_index,
                .draw_arguments ={
                    .VertexCountPerInstance = descriptor.index_count,
                    .InstanceCount = mesh.instance_count,
                    .StartVertexLocation = 0,
                    .StartInstanceLocation = 0,
            },};
            indirect_cmds.push_back(indirect_cmd);

            //m_command_list->SetGraphicsRoot32BitConstants(2,1,&descriptor_index,0);
           // m_command_list->SetGraphicsRoot32BitConstants(3,1,&mesh.instance_index,0);

            //m_command_list->DrawInstanced(descriptor.index_count,mesh.instance_count,0,0);
        }
        UpdateIndirectCommandBuffer(indirect_cmds);

        m_command_list->ExecuteIndirect(m_command_signature.Get(),
            indirect_cmds.size(),
            m_indirect_command_data->GetResource(),
            frame_index*command_buffer_entry.aligned_size_bytes,
            nullptr,
            0);

        D3D12_RESOURCE_BARRIER imgui_transition_to_texture= {};
        imgui_transition_to_texture.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        imgui_transition_to_texture.Transition.pResource = custom_targets[frame_index]->GetResource();
        imgui_transition_to_texture.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        imgui_transition_to_texture.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        imgui_transition_to_texture.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        m_command_list->ResourceBarrier(1,&imgui_transition_to_texture);

        m_command_list->OMSetRenderTargets(1, &rtv_handle, false, &depth_handle);

        ImGui::SetNextWindowSize(ImVec2(m_window_width, m_window_height), ImGuiCond_FirstUseEver);
        ImGui::Begin("Game");
        ImVec2 viewport_size = ImGui::GetContentRegionAvail();
        ImGui::Image(m_descriptor_heap.GetGPUHandle(custom_rt_srv[frame_index]).ptr, viewport_size);
        ImGui::End();

        ImGui::Begin("Properties");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_command_list.Get());

        imgui_transition_to_texture.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        imgui_transition_to_texture.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        m_command_list->ResourceBarrier(1,&imgui_transition_to_texture);

        auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_render_target[frame_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_command_list->ResourceBarrier(1, &barrier2);

    }

    if (auto hr = m_command_list->Close();FAILED(hr))
    {
        std::cout << "Failed to close command list in render loop. ERROR:" << std::hex << hr << std::endl;
        return false;
    }
    return true;
}

void D3D12Context::FlushDevice() {
    auto gpu_flush_value = ++fence_value[frame_index];

    if (auto hr = m_command_queue->Signal(m_fence[frame_index].Get(),gpu_flush_value); FAILED(hr)) {
        std::cout << "Failed to signal fence for flush. ERROR:" << std::hex << hr << std::endl;
    }
    if (auto hr = m_fence[frame_index]->SetEventOnCompletion(gpu_flush_value, fence_event);FAILED(hr)) {
        std::cout << "Failed to complete event for flush. ERROR:" << std::hex << hr << std::endl;
    }
    WaitForSingleObject(fence_event, INFINITE);
}


D3D12Context::~D3D12Context() {
    WaitForPreviousFrame();
    for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
    {
        fence_value[i]++;
        m_command_queue->Signal(m_fence[i].Get(), fence_value[i]);
    }
    for (UINT i = 0; i < FRAME_BUFFER_COUNT; ++i)
    {
        if (m_fence[i]->GetCompletedValue() < fence_value[i])
        {
            m_fence[i]->SetEventOnCompletion(fence_value[i], fence_event);
            WaitForSingleObject(fence_event, INFINITE);
        }
    }
    custom_targets.clear();
    m_depth_buffer.clear();
    vertex_buffers.clear();
    index_buffers.clear();
    m_frame_data.Reset();
    m_instance_data.Reset();
    m_gpu_resources.Clear();
    m_descriptor_heap.Clear();

    TracyD3D12Destroy(m_tracy_context)
    ImGui_ImplDX12_Shutdown();
    CloseHandle(fence_event);


#ifdef _DEBUG
    D3D12DebugLayer::Get().Shutdown();
#endif

}

