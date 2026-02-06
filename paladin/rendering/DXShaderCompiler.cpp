//
// Created by jalr on 31-01-2026.
//

///RESOURCES:
///https://simoncoenen.com/blog/programming/graphics/DxcCompiling
///https://www.wihlidal.com/blog/pipeline/2018-09-16-dxil-signing-post-compile/
#include "DXShaderCompiler.h"

#include <codecvt>

DXShaderCompiler::DXShaderCompiler()
{
    PALADIN_SCOPED_CPU_PROFILE("DXShaderCompiler init", ProfileColors::Red);
    if (auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library)); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Unable to create DxcLibrary",hr))
    }
    if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Unable to create DXcCompiler",hr))
    }
    if (auto hr = DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&m_validator)); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Unable to create DxcValidator.", hr))
    }
    if (auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Unable to load DxcUtils.",hr))
    }

    if (auto hr = m_utils->CreateDefaultIncludeHandler(&m_includeHandler); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Unable to create DxcIncludeHandler.",hr))
    }
}

bool DXShaderCompiler::LoadShader(DXShader& shader)
{
    auto s = "Compiling:"+ConvertWString(shader.shader_input_file);
    PALADIN_SCOPED_CPU_PROFILE(s.data(),ProfileColors::Red);
    shader.last_changed = std::filesystem::last_write_time(shader.file_path);
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> shader_encoding = nullptr;

    if (auto hr = m_library->CreateBlobFromFile(shader.file_path.c_str(),&code_page,shader_encoding.GetAddressOf()); FAILED(hr))
    {
        PALADIN_LOG(ERR, ErrorResult("Unable to create source blob for: " + ConvertWString(shader.shader_input_file),hr))
        return false;
    }

    std::vector<std::wstring> wide_strings;
    std::vector<DxcDefine> defines;

    DxcBuffer source_buffer = {};
    source_buffer.Ptr = shader_encoding->GetBufferPointer();
    source_buffer.Size = shader_encoding->GetBufferSize();
    source_buffer.Encoding = 0;


    std::vector<LPCWSTR> arguments = {};
    arguments.push_back(L"-E"); // Entrypoint
    arguments.push_back(shader.entry_point.c_str());

    arguments.push_back(L"-T"); // Shader profile
    switch (shader.type)
    {
    case VertexShader:
        arguments.push_back(L"vs_6_6");
        break;
    case FragmentShader:
        arguments.push_back(L"ps_6_6");
        break;
    case NONE:
        PALADIN_LOG(WARN, ConvertWString(shader.shader_input_file) + " does not have a specified shader type")
        return false;
        break;
    }


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
            PALADIN_LOG(ERR, ErrorResult("Failed to compile shader : "+ ConvertWString(shader.shader_input_file),hr))
            Microsoft::WRL::ComPtr<IDxcBlobEncoding> error_encoding = nullptr;
            if (compilation_result)
            {
                auto error = compilation_result->GetErrorBuffer(error_encoding.GetAddressOf());
                if (SUCCEEDED(error) && error_encoding)
                {
                    std::string s = static_cast<const char*>(error_encoding->GetBufferPointer());
                    PALADIN_LOG(ERR, "Error Buffer:" + s );
                }
            }
        }
        else
        {
            Microsoft::WRL::ComPtr<IDxcBlob> compiled_shader_code = nullptr;
            if (hr = compilation_result->GetResult(&compiled_shader_code); SUCCEEDED(hr))
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

                Microsoft::WRL::ComPtr<IDxcBlob> reflection_data = nullptr;

                Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection = nullptr;

                Microsoft::WRL::ComPtr<IDxcContainerReflection> reflection_container = nullptr;

                DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&reflection_container));

                reflection_container->Load(compiled_shader_code.Get());

                UINT32 part_index = 0;

                reflection_container->FindFirstPartKind(DXC_PART_DXIL,&part_index);
                reflection_container->GetPartReflection(part_index, IID_PPV_ARGS(&reflection));

                D3D12_SHADER_DESC shader_desc ={};

                if (auto hr = reflection->GetDesc(&shader_desc); FAILED(hr))
                {
                    PALADIN_LOG(ERR,ErrorResult("Unable to get reflection description for:"+ConvertWString(shader.shader_input_file),hr));
                }

                shader.GetCompiledShader() = compiled_shader_code.Get();
                return true;
            }
        }
    }
    else if (FAILED(hr))
    {
        std::wcout << "Unable to compile shader: "<< shader.shader_input_file << " ERROR: " << std::hex << hr << std::endl;
    }

    return false;
}


