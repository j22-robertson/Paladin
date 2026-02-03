//
// Created by James Robertson on 29/01/2026.
//

#include "D3D12DebugLayer.h"


bool D3D12DebugLayer::Init() {
    PALADIN_LOG(INFO, "Enabling D3D12DebugLayer")

#ifdef _DEBUG
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug)))) {
        m_d3d12Debug->EnableDebugLayer();
#ifdef PIX_ENABLE
        PALADIN_LOG(INFO, "PIX Enabled: Setting GPU validation to false")
        m_d3d12Debug->SetEnableGPUBasedValidation(false);
#elif TRACY_ENABLE
        PALADIN_LOG(INFO, "Tracy Enabled: Setting GPU validation to false")
        m_d3d12Debug->SetEnableGPUBasedValidation(false);
#else
        PALADIN_LOG(INFO, "PIX Disabled: Enable GPU Based validation")
        m_d3d12Debug->SetEnableGPUBasedValidation(true);
#endif

        
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_dred_settings))))
        {
            m_dred_settings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            m_dred_settings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            if (SUCCEEDED(DXGIGetDebugInterface1(0,IID_PPV_ARGS(&m_dxgiDebug)))) {
                m_dxgiDebug->EnableLeakTrackingForThread();
                return true;
            }
        }
    }


#endif
    return false;
}
void D3D12DebugLayer::Shutdown() {
   Microsoft::WRL::ComPtr<ID3D12DeviceRemovedExtendedData2> m_dred = nullptr;
#ifdef _DEBUG
    if (m_dxgiDebug) {
        OutputDebugStringW(L"DXGI Debug Report Living Device Objects:\n");
        m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_DETAIL|DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    }
#endif

}