#include <iostream>
#include "application/RenderApplication.h"
#include <memory>
#include <WinPixEventRuntime/pix3.h>




extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = PALADIN_AGILITY_SDK_VERSION; }
extern "C" { __declspec(dllexport) extern const char8_t* D3D12SDKPath = u8".\\D3D12\\"; }
int main()
{

    auto app = std::make_unique<RenderApplication>();
    app->run();
    return 0;
}