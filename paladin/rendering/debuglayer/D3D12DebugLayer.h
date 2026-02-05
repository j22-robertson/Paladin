//
// Created by James Robertson on 29/01/2026.
//

#ifndef PALADIN_D3D12DEBUGLAYER_H
#define PALADIN_D3D12DEBUGLAYER_H
#include <wrl/client.h>
#include <d3dx12/d3dx12.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include "Logger.h"

class D3D12DebugLayer {
public:
    bool Init();

    static void LogDebugMessages(ID3D12Device8* device);
    void Shutdown();


private:
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug6> m_d3d12Debug = nullptr;
    Microsoft::WRL::ComPtr<IDXGIDebug1> m_dxgiDebug = nullptr;

    //https://learn.microsoft.com/en-us/windows/win32/direct3d12/use-dred
    Microsoft::WRL::ComPtr<ID3D12DeviceRemovedExtendedDataSettings2> m_dred_settings = nullptr;
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