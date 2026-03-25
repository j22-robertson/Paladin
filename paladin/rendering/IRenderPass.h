//
// Created by James Robertson on 02/03/2026.
//

#ifndef PALADIN_RENDERPASS_H
#define PALADIN_RENDERPASS_H
#include "d3d12.h"
#include "Scene.h"
class GPUResourceRegistry;
struct InstanceData {
    std::uint32_t heap_offset;
    std::uint32_t frame_offset;
    std::uint32_t id_heap_index;
    std::uint32_t id_frame_offset;
    std::uint32_t instance_offset;
};

class IRenderPass {
public:
    IRenderPass() = default;
    virtual ~IRenderPass() {};
    virtual void Execute(ID3D12GraphicsCommandList8* command_list, GPUResourceRegistry& registry, RenderFrameData& data,std::uint32_t& current_offset) = 0;

private:
};

#endif //PALADIN_RENDERPASS_H