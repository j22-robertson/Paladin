//
// Created by jalr on 05-03-2026.
//

#include "DescriptorHeapManager.h"


bool DescriptorHeapManager::CreateDescriptorHeap(ID3D12Device* device, std::uint32_t hash, D3D12_DESCRIPTOR_HEAP_DESC description)
{
    if (hash_to_heap.contains(hash))
    {
        PALADIN_LOG(WARN,"Heap already exists");
        return false;
    }
    if (auto heap_creation_result = DescriptorHeap::Init(device,description); heap_creation_result.has_value())
    {
        hash_to_heap.insert({hash,std::move(heap_creation_result.value())});
        return true;
    }
    return false;
}
 ViewHandle DescriptorHeapManager::CreateShaderResourceView(ID3D12Device* device, std::uint32_t hash,ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* desc )
{
        if (auto heap = Get(hash);heap!=nullptr)
        {
            const std::uint32_t index = heap->DescriptorCount();

            auto cpu_start = heap->StartCPU();
            const auto descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            cpu_start.ptr += descriptor_size*heap->DescriptorCount();
            device->CreateShaderResourceView(resource, desc,cpu_start);
            heap->IncrementDescriptorCount();
            return {index, hash,DescriptorHeapType::CBV_SRV_UAV};
        }
        return {};
}

ViewHandle DescriptorHeapManager::CreateRenderTargetView(ID3D12Device* device, std::uint32_t hash,ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* desc){
        if (auto heap= Get(hash); heap!=nullptr)
        {
            const std::uint32_t index = heap->DescriptorCount();

            auto cpu_start = heap->StartCPU();
            const auto descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            cpu_start.ptr += descriptor_size*heap->DescriptorCount();
            device->CreateRenderTargetView(resource, desc,cpu_start);
            heap->IncrementDescriptorCount();
            return {index, hash,DescriptorHeapType::RTV};
        }
        return {};
}

    ViewHandle DescriptorHeapManager::CreateDepthStencilView(ID3D12Device* device, std::uint32_t hash,ID3D12Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC* desc)
    {
        if (auto heap = Get(hash); heap!=nullptr)
        {
            const std::uint32_t index = heap->DescriptorCount();
            const auto dest = heap->WriteDest();
            device->CreateDepthStencilView(resource, desc,dest);
            heap->IncrementDescriptorCount();
            return {index, hash,DescriptorHeapType::DSV};
        }
        return {};
    }

ViewHandle DescriptorHeapManager::CreateConstantBufferView(ID3D12Device* device, std::uint32_t hash,D3D12_CONSTANT_BUFFER_VIEW_DESC* desc)
{
    if (auto heap= Get(hash); heap!=nullptr)
    {

        const std::uint32_t index = heap->DescriptorCount();
        const auto dest = heap->WriteDest();
        device->CreateConstantBufferView(desc,dest);
        heap->IncrementDescriptorCount();
        return {index, hash,DescriptorHeapType::CBV_SRV_UAV};
    }
    return {};
}

ID3D12DescriptorHeap* DescriptorHeapManager::GetDescriptorHeap(const std::uint32_t hash) const
{
    return hash_to_heap.at(hash).GetDescriptorHeap();
}

DescriptorHeap* DescriptorHeapManager::Get(const std::uint32_t hash)
{
    if (hash_to_heap.contains(hash))
    {
        return &hash_to_heap.at(hash);
    }
    return nullptr;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapManager::GetCPUHandle(const ViewHandle& handle)
{
    if (handle.type == DescriptorHeapType::UNINITIALIZED)
    {
        return {};
        PALADIN_LOG(WARN,"DescriptorHeapManager:: GetCPUHandle type is undefined");
    }
    if (const auto heap = Get(handle.heap_identifier); heap!=nullptr)
    {
        auto start_handle = heap->StartCPU();
        start_handle.ptr += handle.index*heap->DescriptorIncrementSize();
        return start_handle;
    }
    return {};
}


D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapManager::GetGPUHandle(const ViewHandle& handle)
{
    if (handle.type == DescriptorHeapType::UNINITIALIZED)
    {
        return {};
        PALADIN_LOG(WARN,"DescriptorHeapManager:: GetCPUHandle type is undefined");
    }
    if (const auto heap = Get(handle.heap_identifier); heap!=nullptr)
    {
        auto start_handle = heap->StartGPU();
        start_handle.ptr += handle.index*heap->DescriptorIncrementSize();
        return start_handle;
    }
    return {};
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapManager::GetStartCPUHandle(const std::uint32_t hash) const
{
    if (hash_to_heap.contains(hash))
    {
        return hash_to_heap.at(hash).StartCPU();
    }
    PALADIN_LOG(WARN,"DescriptorHeapManager::GetStartCPUHandle cannot find hash:"+std::to_string(hash));
    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapManager::GetStartGPUHandle(const std::uint32_t hash) const
{
    if (hash_to_heap.contains(hash))
    {
        return hash_to_heap.at(hash).StartGPU();
    }
    PALADIN_LOG(WARN,"DescriptorHeapManager::GetStartCPUHandle cannot find hash:"+std::to_string(hash));
    return {};
}

void DescriptorHeapManager::Clear()
{
    hash_to_heap.clear();
    hash_to_name.clear();
}
