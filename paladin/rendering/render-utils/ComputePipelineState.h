//
// Created by jalr on 03-07-2026.
//

#ifndef PALADIN_COMPUTEPIPELINESTATE_H
#define PALADIN_COMPUTEPIPELINESTATE_H
#include "DXShader.h"
#include "d3dx12/d3dx12_core.h"

class ComputePipelineState
{
public:
    ComputePipelineState(DXShader* shader,ID3D12RootSignature*  root_signature){
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc  = {};
        desc.CS = shader->GetBytecode();
        desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        desc.pRootSignature = root_signature;
        desc.NodeMask  = 0;
    }

private:
    D3D12_COMPUTE_PIPELINE_STATE_DESC compute_desc;

};

#endif //PALADIN_COMPUTEPIPELINESTATE_H
