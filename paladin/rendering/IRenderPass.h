//
// Created by James Robertson on 02/03/2026.
//

#ifndef PALADIN_RENDERPASS_H
#define PALADIN_RENDERPASS_H
#include "d3d12.h"
#include "Scene.h"
class GPUResourceRegistry;
struct PerFrameData{
    std::uint32_t frame_index;
    std::uint32_t ib_buffer_bytes;
    std::uint32_t ib_heap_index;
    std::uint32_t draw_id_buffer_bytes;
    std::uint32_t draw_id_heap_index;

    std::uint32_t vertex_heap_index;
    std::uint32_t index_heap_index;
    std::uint32_t descriptor_heap_index;
};
class IRenderPass {
public:
    IRenderPass() = default;
    virtual ~IRenderPass() {};
    virtual void Execute(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry,FrameUploadData &frame_data) = 0;

private:
};

#endif //PALADIN_RENDERPASS_H