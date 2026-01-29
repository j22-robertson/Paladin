//
// Created by James Robertson on 29/01/2026.
//

#ifndef PALADIN_D3D12DEBUGLAYER_H
#define PALADIN_D3D12DEBUGLAYER_H
#include <wrl/client.h>
#include <d3dx12/d3dx12.h>
#include <dxgidebug.h>

class D3D12DebugLayer {
public:
    bool Init();
    void Shutdown();


private:
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug6> m_d3d12Debug;
    Microsoft::WRL::ComPtr<IDXGIDebug1> m_dxgiDebug;
#endif

public:
    D3D12DebugLayer(const D3D12DebugLayer&) = delete;
    D3D12DebugLayer& operator=(const D3D12DebugLayer&) = delete;
    D3D12DebugLayer() = default;
    static D3D12DebugLayer& Get() {
        static D3D12DebugLayer instance;
        return instance;
    }
};


#endif //PALADIN_D3D12DEBUGLAYER_H