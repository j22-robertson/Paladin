//
// Created by James Robertson on 01/03/2026.
//

#ifndef PALADIN_PIPELINESTATE_H
#define PALADIN_PIPELINESTATE_H
#include <vector>

#include "DXShader.h"
#include "d3dx12/d3dx12_core.h"

class PipelineState {
public:

    PipelineState(DXShader& vertex_shader, DXShader& fragment_shader, ID3D12RootSignature* signature) {

        gpu_pso = {};
        input_layout_desc ={};

        vertex_layout = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
        input_layout_desc.NumElements = vertex_layout.size();
        input_layout_desc.pInputElementDescs = vertex_layout.data();

        sample_desc.Count = 1;
        sample_desc.Quality=0;

        gpu_pso.InputLayout = input_layout_desc;
        gpu_pso.pRootSignature =signature;
        gpu_pso.VS = vertex_shader.GetBytecode();
        gpu_pso.PS = fragment_shader.GetBytecode();
        gpu_pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        gpu_pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        gpu_pso.SampleDesc = sample_desc;
        gpu_pso.SampleMask = 0xfffffff;
        gpu_pso.RasterizerState = CD3DX12_RASTERIZER_DESC2(D3D12_DEFAULT);
        gpu_pso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        gpu_pso.NumRenderTargets = 1;
        gpu_pso.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        gpu_pso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC2(D3D12_DEFAULT);
        gpu_pso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        gpu_pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC gpu_pso;
    D3D12_INPUT_LAYOUT_DESC input_layout_desc;
    std::vector<D3D12_INPUT_ELEMENT_DESC> vertex_layout;
    DXGI_SAMPLE_DESC sample_desc;
};

#endif //PALADIN_PIPELINESTATE_H