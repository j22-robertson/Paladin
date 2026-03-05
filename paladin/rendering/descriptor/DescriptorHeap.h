//
// Created by jalr on 05-03-2026.
//

#ifndef PALADIN_DESCRIPTORHEAP_H
#define PALADIN_DESCRIPTORHEAP_H
#include <optional>
#include <utility>
#include <wrl/client.h>

#include "d3d12.h"

class DescriptorHeap {
public:
    static std::optional<DescriptorHeap>  Init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC desc);
    D3D12_CPU_DESCRIPTOR_HANDLE StartCPU() const;
    D3D12_CPU_DESCRIPTOR_HANDLE WriteDest() const;
    D3D12_GPU_DESCRIPTOR_HANDLE StartGPU() const;
    std::uint32_t DescriptorCount() const;
    std::uint32_t DescriptorIncrementSize() const;
    void IncrementDescriptorCount();
    ~DescriptorHeap() = default;
private:
    DescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _heap, const D3D12_DESCRIPTOR_HEAP_DESC& _desc, std::uint32_t _ds);
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    std::uint32_t descriptor_count = 0;
    std::uint32_t descriptor_size = 0;
};


#endif //PALADIN_DESCRIPTORHEAP_H