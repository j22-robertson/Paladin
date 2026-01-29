//
// Created by James Robertson on 29/01/2026.
//

#include "D3D12DebugLayer.h"
#include <dxgi1_3.h>

bool D3D12DebugLayer::Init() {
#ifdef _DEBUG
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug)))) {
        m_d3d12Debug->EnableDebugLayer();
        if (SUCCEEDED(DXGIGetDebugInterface1(0,IID_PPV_ARGS(&m_dxgiDebug)))) {
            m_dxgiDebug->EnableLeakTrackingForThread();
            return true;
        }
    }


#endif
    return false;
}
void D3D12DebugLayer::Shutdown() {
#ifdef _DEBUG
    if (m_dxgiDebug) {
        OutputDebugStringW(L"DXGI Debug Report Living Device Objects:\n");
        m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL|DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    }
#endif

}