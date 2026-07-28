//
// Created by James Robertson on 02/03/2026.
//

#ifndef PALADIN_FORWARDPASS_H
#define PALADIN_FORWARDPASS_H
#include <span>

#include "IRenderPass.h"
#include "Scene.h"
#include "asset/Mesh.h"

class ForwardPass: public IRenderPass {
public:
    //void Execute(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry, RenderFrameData& data,std::uint32_t& current_offset) override;
    void Execute(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry,FrameUploadData &frame_data) override;
    void ExecuteVertexPull(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry,FrameUploadData &frame_data, std::span<MeshDescriptor> mesh_descriptors);
private:
};


#endif //PALADIN_FORWARDPASS_H