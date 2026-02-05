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

void D3D12DebugLayer::LogDebugMessages(ID3D12Device8 *device) {
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> debug_info;
    if (auto hr = device->QueryInterface(IID_PPV_ARGS(&debug_info)); SUCCEEDED(hr)) {
        auto message_count = debug_info->GetNumMessagesAllowedByStorageFilter();
        for (int i = 0; i < message_count; i++) {
            D3D12_MESSAGE debug_message = {};
            SIZE_T message_length = 0;
            if (auto hr = debug_info->GetMessageA(i, nullptr, &message_length); FAILED(hr)) {
                PALADIN_LOG(ERR, ErrorResult("Unable to retrieve debug message length", hr));
            }
            else {
                if (auto hr = debug_info->GetMessageA(i, &debug_message, &message_length); FAILED(hr)) {
                    PALADIN_LOG(ERR, ErrorResult("Unable to retrieve debug message", hr));
                }
                else {
                    PALADIN_LOG(DEBUG, "DX12:"+std::string(debug_message.pDescription, debug_message.DescriptionByteLength-1));
                }
            }
        }
    }
    else {
        PALADIN_LOG(ERR, ErrorResult("Unable to get debug log", hr));
    }
#endif
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
