//
// Created by James Robertson on 29/01/2026.
//

#ifndef PALADIN_D3DX12CONTEXT_H
#define PALADIN_D3DX12CONTEXT_H
#include <d3dx12/d3dx12.h>
class D3D12Context {
public:
    D3D12Context (const D3D12Context &) = delete;
    D3D12Context& operator= (const D3D12Context &) = delete;
    D3D12Context() = default;
    ~D3D12Context();
};
#endif //PALADIN_D3DX12CONTEXT_H