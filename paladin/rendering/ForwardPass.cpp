//
// Created by James Robertson on 02/03/2026.
//
#include "ForwardPass.h"

#include "resource/GPUResourceRegistry.h"

void ForwardPass::Execute(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry, RenderFrameData& data) {
    PALADIN_SCOPED_CPU_PROFILE("ForwardPass::Execute", ProfileColors::Green);

    uint32_t current_offset = 0;
    std::uint32_t instance_buffer_id =data.instance_buffer_id;
    auto instance_data = InstanceData{.heap_offset = instance_buffer_id,.frame_offset = data.frame_index * data.aligned_bytes_per_buffer};
    command_list->SetGraphicsRoot32BitConstants(3,2,&instance_data,0);
    for (auto& batch : data.batches) {


        auto model = registry.GetOpaque<GPUModel>(batch.handle);

        for (int i = 0; i < model->mesh_handles.size(); i++) {
            auto mesh_handle = model->mesh_handles[i];
            auto mesh = registry.Get<GPUMesh>(mesh_handle);
            auto material = registry.GetOpaque<GPUMaterial>(mesh->material);
            auto albedo = registry.Get<GPUTexture2D>(material->albedo);

            std::uint32_t albedo_index = albedo->srv_descriptor_index;


            command_list->SetGraphicsRoot32BitConstants(2,1,&albedo_index,0);

            command_list->IASetVertexBuffers(0,1,&mesh->vertex_buffer_view);
            command_list->IASetIndexBuffer(&mesh->index_buffer_view);
            command_list->DrawIndexedInstanced(mesh->indices, batch.transforms.size(), 0, 0, current_offset);

        }
        current_offset += batch.transforms.size();
    }
}
