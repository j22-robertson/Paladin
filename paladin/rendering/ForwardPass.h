//
// Created by James Robertson on 02/03/2026.
//

#ifndef PALADIN_FORWARDPASS_H
#define PALADIN_FORWARDPASS_H
#include "IRenderPass.h"
#include "Scene.h"

struct IndirectCommand
{
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
    D3D12_DRAW_INDEXED_ARGUMENTS draw_indexed_arguments;
    std::uint32_t padding[3];
};

class ForwardPass: public IRenderPass {
public:
    void Execute(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry, RenderFrameData& data) override;
    void ExecuteIndirect(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry, RenderFrameData& data, std::vector<IndirectCommand>& draw_commands);
private:
};


#endif //PALADIN_FORWARDPASS_H