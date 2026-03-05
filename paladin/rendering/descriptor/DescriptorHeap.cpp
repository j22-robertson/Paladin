//
// Created by jalr on 05-03-2026.
//

#include "DescriptorHeap.h"
#include "Logger.h"

std::optional<DescriptorHeap> DescriptorHeap::Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC _desc)
{
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> new_heap = nullptr;
    if (auto hr = device->CreateDescriptorHeap(&_desc,IID_PPV_ARGS(&new_heap)); FAILED(hr)) {
        PALADIN_LOG(ERR, ErrorResult("Failed to create descriptor heap", hr))
        return std::nullopt;
    }
    auto size = device->GetDescriptorHandleIncrementSize(_desc.Type);
    return std::make_optional<DescriptorHeap>(DescriptorHeap(std::move(new_heap), _desc,size));
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::StartCPU() const
{
    return descriptor_heap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::WriteDest() const
{
    auto write_handle = descriptor_heap->GetCPUDescriptorHandleForHeapStart();
    write_handle.ptr += descriptor_size*descriptor_count;
    return write_handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::StartGPU() const
{
    return descriptor_heap->GetGPUDescriptorHandleForHeapStart();
}

std::uint32_t DescriptorHeap::DescriptorCount() const
{
    return descriptor_count;
}

std::uint32_t DescriptorHeap::DescriptorIncrementSize() const
{
    return descriptor_size;
}

void DescriptorHeap::IncrementDescriptorCount()
{
    descriptor_count+=1;
}

ID3D12DescriptorHeap* DescriptorHeap::GetDescriptorHeap() const
{
    return descriptor_heap.Get();
}

DescriptorHeap::DescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _heap, const D3D12_DESCRIPTOR_HEAP_DESC& _desc, const std::uint32_t _ds)
{
    descriptor_heap = std::move(_heap);
    desc = _desc;
    descriptor_size = _ds;
}
