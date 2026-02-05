//
// Created by James Robertson on 29/01/2026.
//
#include "D3D12Context.h"

#include <dxcapi.h>
#include <iostream>
#include <ostream>

#include "imgui_impl_glfw.h"

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


    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter = nullptr;

    Microsoft::WRL::ComPtr<IDXGIFactory2> m_dxgi_factory = nullptr;
    if (auto hr = CreateDXGIFactory2(factory_flags,IID_PPV_ARGS(&m_dxgi_factory)); FAILED(hr)) {
        PALADIN_LOG(INFO, "Created DXGI Factory")
    }

    bool adapter_found = false;
    bool able_to_support_feature = true;
    for (UINT adapter_index= 0; able_to_support_feature; adapter_index++ )
    {
        if (auto hr =  m_dxgi_factory->EnumAdapters1(adapter_index, &adapter); FAILED(hr)) {
            PALADIN_LOG(ERR, ErrorResult("Failed to match feature level." , hr))
            able_to_support_feature = false;
        }

        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, _uuidof(ID3D12Device), nullptr))) {
            std::wstring gpu_name = desc.Description;
            PALADIN_LOG(INFO, "Found Graphics Device: " + ConvertWString(gpu_name))
            adapter_found = true;
            break;
        }
    }

    if (adapter_found) {
        PALADIN_LOG(INFO, "Found DXGI Device Adapter")
        if (const auto hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_device)); SUCCEEDED(hr))
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

    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};

    //TODO: Change num descriptors
    srv_heap_desc.NumDescriptors =1;
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (auto hr = m_device->CreateDescriptorHeap(&srv_heap_desc,IID_PPV_ARGS(&m_srv_descriptor_heap)); SUCCEEDED(hr)) {
        PALADIN_LOG(INFO, "Created srv descriptor heap")
    }
    else if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create srv descriptor heap", hr))
    }


    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};

    rtv_heap_desc.NumDescriptors = FRAME_BUFFER_COUNT;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (auto hr = m_device->CreateDescriptorHeap(&rtv_heap_desc,IID_PPV_ARGS(&m_rtv_descriptor_heap)); SUCCEEDED(hr)) {
        PALADIN_LOG(INFO, "Successfully created rtv descriptor heap")
    }
    else if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create rtv descriptor heap", hr))
    }

    auto rtv_descriptor_size =  m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv_cpu_handle(m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0 ; i < FRAME_BUFFER_COUNT ; i++) {
        if (auto hr = m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_render_target[i])); SUCCEEDED(hr)) {
            PALADIN_LOG(INFO, "Created render target")
        }
        else if (FAILED(hr)) {
            PALADIN_LOG(ERR, ErrorResult("Failed to create render target", hr))
        }
        m_device->CreateRenderTargetView(m_render_target[i].Get(), nullptr, rtv_cpu_handle);
        rtv_cpu_handle.ptr += rtv_descriptor_size;
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


    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.Init(0,nullptr,0,nullptr,D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signature = nullptr;

    if (auto hr = D3D12SerializeRootSignature(&root_signature_desc,D3D_ROOT_SIGNATURE_VERSION_1_0,&signature,nullptr); SUCCEEDED(hr))
    {
        PALADIN_LOG(INFO, "Successfully serialized root signature")
    }
    else if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to serialize root signature",hr))
        return;
    }

    if (auto hr = m_device->CreateRootSignature(0,signature->GetBufferPointer(),signature->GetBufferSize(),IID_PPV_ARGS(&m_root_signature)); SUCCEEDED(hr))
    {
        PALADIN_LOG(INFO, "Successfully created root signature");
    }
    else if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create root signature", hr))
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
    fragment_shader = DXShader(FragmentShader, L"fs.frag",L"main");
    if (!m_shader_compiler.LoadShader(fragment_shader))
    {
        PALADIN_LOG(ERR, "Unable to load: " + ConvertWString(fragment_shader.shader_input_file))
        return;
    }
    fragment_shader_bytecode = fragment_shader.GetBytecode();




    D3D12_INPUT_ELEMENT_DESC input_layout[] ={
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,sizeof(float)*3,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
    };

    D3D12_INPUT_LAYOUT_DESC input_layout_desc ={};

    input_layout_desc.NumElements = sizeof(input_layout)/sizeof(D3D12_INPUT_ELEMENT_DESC);
    input_layout_desc.pInputElementDescs = input_layout;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};

    pso_desc.InputLayout = input_layout_desc;
    pso_desc.pRootSignature = m_root_signature.Get();
    pso_desc.VS = vertex_shader_bytecode;
    pso_desc.PS = fragment_shader_bytecode;
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso_desc.SampleDesc = sample_desc;
    pso_desc.SampleMask = 0xfffffff;
    pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC2(D3D12_DEFAULT);
    pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pso_desc.NumRenderTargets = 1;
    pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC2(D3D12_DEFAULT);
    pso_desc.DepthStencilState.DepthEnable = false;

    if (auto hr = m_device->CreateGraphicsPipelineState(&pso_desc,IID_PPV_ARGS(&m_pipeline_state)); SUCCEEDED(hr))
    {
        PALADIN_LOG(INFO, "Successfully created pipeline state")
    }
    else if (FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Failed to create pipeline state.", hr))
    }

    int vertex_buffer_size = sizeof(triangle_vertices);

    auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resource_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);

    if (auto hr = m_device->CreateCommittedResource(&heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resource_buffer_desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&triangle_vertex_buffer));FAILED(hr)) {
            PALADIN_LOG(ERR,ErrorResult("Failed to aallocate memory for Vertex Buffer", hr))
            return;
        }

    triangle_vertex_buffer->SetName(L"Triangle VBO");

    auto upload_buffer_size =vertex_buffer_size;

    auto temp_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto temp_resource_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(upload_buffer_size);

    if (auto hr = m_device->CreateCommittedResource(&temp_heap_properties,
          D3D12_HEAP_FLAG_NONE,
          &temp_resource_buffer_desc,
          D3D12_RESOURCE_STATE_GENERIC_READ,
          nullptr,
          IID_PPV_ARGS(&temporary_upload_heap)); FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create upload heap", hr))
        return;
    }

    temporary_upload_heap->SetName(L"Temporary Upload Heap");

    D3D12_SUBRESOURCE_DATA vertex_data = {};

    vertex_data.pData = reinterpret_cast<BYTE*>(triangle_vertices);
    vertex_data.RowPitch = vertex_buffer_size;
    vertex_data.SlicePitch = vertex_buffer_size;

    UpdateSubresources(m_command_list.Get(),
        triangle_vertex_buffer.Get(),
        temporary_upload_heap.Get(),
        0,
        0,
        1,
        &vertex_data);
    auto vertex_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(triangle_vertex_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    m_command_list->ResourceBarrier(1, &vertex_transition_barrier);
    m_command_list->Close();

    ID3D12CommandList* cmd_lists[] = { m_command_list.Get()};
    m_command_queue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);
    fence_value[frame_index]++;
    auto hr =  m_command_queue->Signal(m_fence[frame_index].Get(), fence_value[frame_index]);

    if (FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to signal fence.", hr))
    }
    vertex_buffer_view.BufferLocation = triangle_vertex_buffer->GetGPUVirtualAddress();
    vertex_buffer_view.SizeInBytes = vertex_buffer_size;
    vertex_buffer_view.StrideInBytes = sizeof(Vertex);


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

    imgui_init_info.SrvDescriptorHeap = m_srv_descriptor_heap.Get();

    CD3DX12_CPU_DESCRIPTOR_HANDLE srv_handle(m_srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),frame_index,m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    ImGui_ImplDX12_Init(m_device.Get(),
        FRAME_BUFFER_COUNT,DXGI_FORMAT_R8G8B8A8_UNORM,
        m_srv_descriptor_heap.Get(),
        srv_handle,
        m_srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart());

#ifdef TRACY_ENABLE
    m_tracy_context = TracyD3D12Context(m_device.Get(),m_command_queue.Get())
    if (m_tracy_context == nullptr) {
        PALADIN_LOG(ERR, "Unable to create tracy context.")
    }
    else {
        PALADIN_LOG(INFO,"Tracy context created.");
    }
#endif

    //imgui_init_info.SrvDescriptorHeap =
    //ImGui_ImplDX12_Init()
}

Microsoft::WRL::ComPtr<ID3D12Resource> D3D12Context::UploadVertices() {
    //TODO: UploadModel takes model data and uploads it to a GPU buffer for access, return handle for use in application
    int vertex_buffer_size = sizeof(triangle_vertices);

    auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resource_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);

    if (auto hr = m_device->CreateCommittedResource(&heap_properties,
        D3D12_HEAP_FLAG_NONE,
        &resource_buffer_desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&triangle_vertex_buffer));FAILED(hr)) {
        std::cout << "Failed to allocate memory for Vertex Buffer. ERROR CODE: "<< std::hex << hr << std::endl;
        return nullptr;
        }

    triangle_vertex_buffer->SetName(L"Triangle VBO");

    auto upload_buffer_size =vertex_buffer_size;

    auto temp_heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto temp_resource_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(upload_buffer_size);

    if (auto hr = m_device->CreateCommittedResource(&temp_heap_properties,
          D3D12_HEAP_FLAG_NONE,
          &temp_resource_buffer_desc,
          D3D12_RESOURCE_STATE_GENERIC_READ,
          nullptr,
          IID_PPV_ARGS(&temporary_upload_heap)); FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create upload buffer.", hr))
        return nullptr;
          }

    temporary_upload_heap->SetName(L"Temporary Upload");

    D3D12_SUBRESOURCE_DATA vertex_data = {};

    vertex_data.pData = reinterpret_cast<BYTE*>(triangle_vertices);
    vertex_data.RowPitch = vertex_buffer_size;
    vertex_data.SlicePitch = vertex_buffer_size;

    UpdateSubresources(m_command_list.Get(),
        triangle_vertex_buffer.Get(),
        temporary_upload_heap.Get(),
        0,
        0,
        1,
        &vertex_data);
    auto vertex_transition_barrier = CD3DX12_RESOURCE_BARRIER::Transition(triangle_vertex_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    m_command_list->ResourceBarrier(1, &vertex_transition_barrier);

    return nullptr;
}

bool D3D12Context::Render()
{
    PALADIN_SCOPED_CPU_PROFILE("D3D12Context::Render", ProfileColors::Red);
    PALADIN_BEGIN_GPU_PROFILE(m_command_queue.Get(), ProfileColors::Green, "D3D12Context::Render")
    if (!WaitForPreviousFrame())
    {
        return false;
    }

    TracyD3D12Collect(m_tracy_context)
    TracyD3D12NewFrame(m_tracy_context)

    {
        if (vertex_shader.NeedsRecompilation()) {
            if (m_shader_compiler.LoadShader(vertex_shader)) {
                PALADIN_LOG(INFO, "Recompiling "+ConvertWString(vertex_shader.shader_input_file))
                DXGI_SAMPLE_DESC sample_desc{};
                sample_desc.Count = 1;
                sample_desc.Quality=0;

                D3D12_INPUT_ELEMENT_DESC input_layout[] ={
                    {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
                    {"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,sizeof(float)*3,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
                };

                D3D12_INPUT_LAYOUT_DESC input_layout_desc ={};

                input_layout_desc.NumElements = sizeof(input_layout)/sizeof(D3D12_INPUT_ELEMENT_DESC);
                input_layout_desc.pInputElementDescs = input_layout;

                D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};

                pso_desc.InputLayout = input_layout_desc;
                pso_desc.pRootSignature = m_root_signature.Get();
                pso_desc.VS = vertex_shader.GetBytecode();
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
                pso_desc.DepthStencilState.DepthEnable = false;

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
                return false;
            }
        }


    }

    {
        PALADIN_SCOPED_CPU_PROFILE("D3D12Context::UpdatePipeline",ProfileColors::Red);
        if (!UpdatePipeline())
        {
            return false;
        }
    }


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

        if (auto hr = m_swap_chain->Present(0,0); FAILED(hr))
        {
            PALADIN_LOG(ERR, ErrorResult("Failed to present swap chain", hr))
            return false;
        }
    }




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

bool D3D12Context::UpdatePipeline()
{

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
        ImGui::ShowDemoWindow();


        m_command_list->RSSetViewports(1, &m_viewport);
        m_command_list->RSSetScissorRects(1,&m_scissor);

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_render_target[frame_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        m_command_list->ResourceBarrier(1,&barrier);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),frame_index,m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
        m_command_list->OMSetRenderTargets(1, &rtv_handle, false, nullptr);
        const float clear_colour[] = { 0.0f,0.2f,0.4f,1.0f };
        m_command_list->ClearRenderTargetView(rtv_handle,clear_colour,0,nullptr);

        m_command_list->SetGraphicsRootSignature(m_root_signature.Get());

        m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_command_list->IASetVertexBuffers(0,1,&vertex_buffer_view);
        m_command_list->DrawInstanced(3,1,0,0);

        ID3D12DescriptorHeap* heaps[] = {m_srv_descriptor_heap.Get() };
        m_command_list->SetDescriptorHeaps(1, heaps);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_command_list.Get());

        auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_render_target[frame_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_command_list->ResourceBarrier(1, &barrier2);

    }

    if (auto hr = m_command_list->Close();FAILED(hr))
    {
        std::cout << "Failed to close command list. ERROR:" << std::hex << hr << std::endl;
        return false;
    }
    return true;
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
    TracyD3D12Destroy(m_tracy_context)
    ImGui_ImplDX12_Shutdown();
    CloseHandle(fence_event);
#ifdef _DEBUG
    D3D12DebugLayer::Get().Shutdown();
#endif

}