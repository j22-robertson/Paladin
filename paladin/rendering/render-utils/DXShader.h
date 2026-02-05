//
// Created by jalr on 31-01-2026.
//

#ifndef PALADIN_DXSHADERFILE_H
#define PALADIN_DXSHADERFILE_H
#include "dxcapi.h"
#include "d3d12.h"
#include <string>
#include <wrl/client.h>
#include <filesystem>

enum ShaderType
{
    VertexShader,
    FragmentShader,
    NONE,
    //TODO: Add more shader types
};
class DXShader
{
    public:
    DXShader() = default;
    DXShader(const ShaderType input_type, const std::wstring& input_file, const std::wstring& _entry_point)
    {
        shader_input_file = input_file;
        //TODO: There is likely a much better way to handle these file paths
        std::filesystem::path root_dir  =  PALADIN_ROOT_DIR;
        file_path =  root_dir / "paladin/rendering/shaders" / input_file;
        type = input_type;
        entry_point = _entry_point;
        last_changed = std::filesystem::last_write_time(file_path);
    }
    bool NeedsRecompilation() {
        auto current_write_time = std::filesystem::last_write_time(file_path);
        auto now = std::filesystem::file_time_type::clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now-current_write_time).count();
        if (duration<100) {
            return false;
        }
        if (current_write_time!=last_changed)[[unlikely]] {
            return true;
        }
        return false;
    }
    D3D12_SHADER_BYTECODE GetBytecode() {
        D3D12_SHADER_BYTECODE shader_bytecode{};
        shader_bytecode.BytecodeLength = compiled_shader->GetBufferSize();
        shader_bytecode.pShaderBytecode = compiled_shader->GetBufferPointer();
        return shader_bytecode;
    }
    Microsoft::WRL::ComPtr<IDxcBlob>& GetCompiledShader()
    {
        return compiled_shader;
    }

    ShaderType type = NONE;
    std::filesystem::path file_path;
    std::wstring entry_point;
    std::wstring shader_input_file;
    std::filesystem::file_time_type last_changed;
private:
    Microsoft::WRL::ComPtr<IDxcBlob> compiled_shader = nullptr;

};

#endif //PALADIN_DXSHADERFILE_H