//
// Created by James Robertson on 02/03/2026.
//
#include "ForwardPass.h"

#include "resource/GPUResourceRegistry.h"
/*
void ForwardPass::Execute(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry, RenderFrameData& data,std::uint32_t& current_offset) {
    PALADIN_SCOPED_CPU_PROFILE("ForwardPass::Execute", ProfileColors::Green);

    std::uint32_t instance_buffer_id =data.instance_buffer_id;

    for (auto& batch : data.batches) {
        auto model = registry.GetOpaque<GPUModel>(batch.handle);
        auto instance_data = InstanceData{.heap_offset = instance_buffer_id,.frame_offset = data.frame_index * data.aligned_bytes_per_buffer,.instance_offset = current_offset};
        command_list->SetGraphicsRoot32BitConstants(3,3,&instance_data,0);
        for (int i = 0; i < model->mesh_handles.size(); i++) {
            auto mesh_handle = model->mesh_handles[i];
            auto mesh = registry.Get<GPUMesh>(mesh_handle);
            auto material = registry.GetOpaque<GPUMaterial>(mesh->material);
            auto albedo = registry.Get<GPUTexture2D>(material->albedo);
            std::uint32_t albedo_index = albedo->srv_handle.index;
            command_list->SetGraphicsRoot32BitConstants(2,1,&albedo_index,0);
            command_list->IASetVertexBuffers(0,1,&mesh->vertex_buffer_view);
            command_list->IASetIndexBuffer(&mesh->index_buffer_view);
            command_list->DrawIndexedInstanced(mesh->indices, batch.transforms.size(), 0, 0, 0);
        }
        current_offset += batch.transforms.size();
    }
}*/

void ForwardPass::Execute(ID3D12GraphicsCommandList8 *command_list, GPUResourceRegistry &registry, FrameUploadData& frame_data) {
    PALADIN_SCOPED_CPU_PROFILE("ForwardPass::ExecuteTest", ProfileColors::Green);
    for (const auto& e : frame_data.visible_meshes) {
        command_list->SetGraphicsRoot32BitConstants(3,1,&e.instance_index,0);
        auto mesh = registry.GetOpaque<GPUMesh>(e.mesh_handle);
        auto material = registry.GetOpaque<GPUMaterial>(mesh->material);
        auto albedo = registry.Get<GPUTexture2D>(material->albedo);
        command_list->SetGraphicsRoot32BitConstants(2,1,&albedo->srv_handle.index,0);
        command_list->IASetVertexBuffers(0,1,&mesh->vertex_buffer_view);
        command_list->IASetIndexBuffer(&mesh->index_buffer_view);
        command_list->DrawIndexedInstanced(mesh->indices, e.instance_count, 0, 0, 0);
    }
}
