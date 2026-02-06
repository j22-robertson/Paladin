//
// Created by jalr on 31-01-2026.
//

#ifndef PALADIN_DXSHADERCOMPILER_H
#define PALADIN_DXSHADERCOMPILER_H
#include <wrl/client.h>
#include <iostream>
#include <filesystem>
#include <dxcapi.h>
#include "Logger.h"
#include "d3d12shader.h"
#include "render-utils/DXShader.h"
#include "profiling/Profiling.h"
class DXShaderCompiler
{
public:
    DXShaderCompiler();
    bool LoadShader(DXShader& shader);

private:
    Microsoft::WRL::ComPtr<IDxcCompiler3> m_compiler = nullptr;
    Microsoft::WRL::ComPtr<IDxcLibrary> m_library = nullptr;
    Microsoft::WRL::ComPtr<IDxcValidator2> m_validator = nullptr;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_includeHandler = nullptr;
    Microsoft::WRL::ComPtr<IDxcUtils> m_utils = nullptr;
    UINT32 code_page = CP_UTF8; //Assuming always UTF8 for now
};

#endif //PALADIN_DXSHADERCOMPILER_H