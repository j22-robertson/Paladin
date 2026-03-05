//
// Created by jalr on 05-03-2026.
//

#ifndef PALADIN_DESCRIPTORHEAPMANAGER_H
#define PALADIN_DESCRIPTORHEAPMANAGER_H
#include <string>
#include <string_view>
#include <unordered_map>

#include "d3d12.h"
#include "DescriptorHeap.h"
#include "Logger.h"
#include "utility/StringHash.h"
enum class DescriptorHeapType : std::uint32_t
{
    UNINITIALIZED = 0,
    CBV_SRV_UAV=1,
    RTV=2,
    DSV=3,
};

struct ViewHandle
{
    // The starting descriptor index
    std::uint32_t index=0;
    // The identifier for the heap the view exists in
    std::uint32_t heap_identifier =0;

    DescriptorHeapType type=DescriptorHeapType::UNINITIALIZED;
};

class DescriptorHeapManager
{
public:
    bool CreateDescriptorHeap(ID3D12Device* device, std::uint32_t hash, D3D12_DESCRIPTOR_HEAP_DESC description);
    ViewHandle CreateShaderResourceView(ID3D12Device* device, std::uint32_t hash,ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* desc = nullptr);
    ViewHandle CreateRenderTargetView(ID3D12Device* device, std::uint32_t hash,ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* desc = nullptr);
    ViewHandle CreateDepthStencilView(ID3D12Device* device, std::uint32_t hash,ID3D12Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC* desc = nullptr);
    ViewHandle CreateConstantBufferView(ID3D12Device* device, std::uint32_t hash,D3D12_CONSTANT_BUFFER_VIEW_DESC* desc = nullptr);

    ID3D12DescriptorHeap* GetDescriptorHeap(std::uint32_t hash) const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(const ViewHandle& handle);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const ViewHandle& handle);

    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCPUHandle(std::uint32_t hash) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGPUHandle(std::uint32_t hash) const;

    void Clear();
private:
    DescriptorHeap*  Get(std::uint32_t hash);
    std::unordered_map<std::uint32_t, DescriptorHeap> hash_to_heap;
    std::unordered_map<std::uint32_t,std::string> hash_to_name;
};


#endif //PALADIN_DESCRIPTORHEAPMANAGER_H