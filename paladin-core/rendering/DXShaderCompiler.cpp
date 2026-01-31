//
// Created by jalr on 31-01-2026.
//

///RESOURCES:
///https://simoncoenen.com/blog/programming/graphics/DxcCompiling
///https://www.wihlidal.com/blog/pipeline/2018-09-16-dxil-signing-post-compile/
#include "DXShaderCompiler.h"

DXShaderCompiler::DXShaderCompiler()
{
    if (auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library)); FAILED(hr))
    {
        std::cout << "Unable to create DxcLibrary ERROR: " << std::hex<<hr << std::endl;
    }

    if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)); FAILED(hr))
    {
        std::cout << "Unable to create DxcCompiler. ERROR: " << std::hex<<hr << std::endl;
    }

    if (auto hr = DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&m_validator)); FAILED(hr))
    {
        std::cout << "Unable to create DxcValidator. ERROR: " << std::hex<<hr << std::endl;
    }
    if (auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)); FAILED(hr))
    {
        std::cout << "Unable to load DxcUtils. ERROR: " << std::hex<<hr << std::endl;
    }

    if (auto hr = m_utils->CreateDefaultIncludeHandler(&m_includeHandler); FAILED(hr))
    {
        std::cout << "Unable to create DxcIncludeHandler. ERROR: " << std::hex<<hr << std::endl;
    }


}

Microsoft::WRL::ComPtr<IDxcBlob> DXShaderCompiler::LoadShader(const std::filesystem::path& shader)
{
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> shader_encoding = nullptr;
    if (auto hr = m_library->CreateBlobFromFile(L"../paladin-core/rendering/shaders/vs.hlsl",&code_page,shader_encoding.GetAddressOf()); FAILED(hr))
    {
        std::cout << "Unable to create source blob for: "<< shader << " ERROR: " << std::hex<<hr << std::endl;
    }

    std::vector<std::wstring> wide_strings;
    std::vector<DxcDefine> defines;

    DxcBuffer source_buffer = {};
    source_buffer.Ptr = shader_encoding->GetBufferPointer();
    source_buffer.Size = shader_encoding->GetBufferSize();
    source_buffer.Encoding = 0;


    std::vector<LPCWSTR> arguments = {};
    arguments.push_back(L"-E"); // Entrypoint
    arguments.push_back(L"main");

    arguments.push_back(L"-T"); // Shader profile
    arguments.push_back(L"vs_6_6");

    arguments.push_back(DXC_ARG_ENABLE_STRICTNESS); // Enable strictness
    arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
#ifdef _DEBUG
    arguments.push_back(DXC_ARG_DEBUG);
    arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
#endif

    Microsoft::WRL::ComPtr<IDxcResult> compiled_shader_result = nullptr;

    Microsoft::WRL::ComPtr<IDxcOperationResult> compilation_result = nullptr;
    if (auto hr = m_compiler->Compile(&source_buffer,arguments.data(),arguments.size(),m_includeHandler.Get(), IID_PPV_ARGS(&compilation_result));SUCCEEDED(hr))
    {
        compilation_result->GetStatus(&hr);
        if (FAILED(hr))
        {
            std::cout << "Failed to compile shader: " << std::hex << hr << std::endl;
            Microsoft::WRL::ComPtr<IDxcBlobEncoding> error_encoding = nullptr;
            if (compilation_result)
            {
                auto error = compilation_result->GetErrorBuffer(error_encoding.GetAddressOf());
                if (SUCCEEDED(error) && error_encoding)
                {
                    std::cout << "Error buffer: " << static_cast<const char*>(error_encoding->GetBufferPointer()) << std::endl;
                }
            }
        }
        else
        {
            Microsoft::WRL::ComPtr<IDxcBlob> compiled_shader_code = nullptr;
            if (hr = compilation_result->GetResult(&compiled_shader_code); FAILED(hr))
            {
                Microsoft::WRL::ComPtr<IDxcOperationResult> validation_result = nullptr;
#ifdef _DEBUG
                if (auto hr = m_validator->ValidateWithDebug(compiled_shader_code.Get(),DxcValidatorFlags_InPlaceEdit,nullptr,&validation_result);FAILED(hr))
                {
                    std::cout << "Validation Failed. Error: " << std::hex << hr << std::endl;
                    validation_result->GetStatus(&hr);
                    if (FAILED(hr))
                    {
                        std::cout << "Failed to validate shader: " << std::hex << hr << std::endl;
                        Microsoft::WRL::ComPtr<IDxcBlobEncoding> validation_error_encoding = nullptr;
                        if (validation_result)
                        {
                            auto error = validation_result->GetErrorBuffer(validation_error_encoding.GetAddressOf());
                            if (SUCCEEDED(error) && validation_error_encoding)
                            {
                                std::cout << "Error buffer: " << static_cast<const char*>(validation_error_encoding->GetBufferPointer()) << std::endl;
                            }
                        }
                    }
                }
#else
                if (auto hr = m_validator->Validate(compiled_shader_code.Get(), DxcValidatorFlags_InPlaceEdit, &validation_result); FAILED(hr))
                {
                    std::cout << "Validation Failed. Error: " << std::hex << hr << std::endl;
                    validation_result->GetStatus(&hr);
                    if (FAILED(hr))
                    {
                        std::cout << "Failed to validate shader: " << std::hex << hr << std::endl;
                        Microsoft::WRL::ComPtr<IDxcBlobEncoding> validation_error_encoding = nullptr;
                        if (validation_result)
                        {
                            auto error = validation_result->GetErrorBuffer(validation_error_encoding.GetAddressOf());
                            if (SUCCEEDED(error) && validation_error_encoding)
                            {
                                std::cout << "Error buffer: " << static_cast<const char*>(validation_error_encoding->GetBufferPointer()) << std::endl;
                            }
                        }
                    }
                }
#endif


                return compiled_shader_code;
            }
        }
    }
    else if (FAILED(hr))
    {
        std::cout << "Unable to compile shader: "<< shader << " ERROR: " << std::hex << hr << std::endl;
    }

}
