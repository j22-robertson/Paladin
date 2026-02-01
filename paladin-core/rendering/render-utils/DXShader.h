//
// Created by jalr on 31-01-2026.
//

#ifndef PALADIN_DXSHADERFILE_H
#define PALADIN_DXSHADERFILE_H
#include <dxcapi.h>
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
    DXShader(const ShaderType input_type, const std::wstring& input_file, const std::wstring& _entry_point)
    {
        shader_input_file = input_file;
        //TODO: There is likely a much better way to handle these file paths
        std::filesystem::path root_dir  =  PALADIN_ROOT_DIR;
        file_path =  root_dir / "paladin-core/rendering/shaders" / input_file;
        type = input_type;
        entry_point = _entry_point;
    }
    Microsoft::WRL::ComPtr<IDxcBlob>& GetCompiledShader()
    {
        return compiled_shader;
    }

    ShaderType type = NONE;
    std::wstring shader_input_file;
    std::filesystem::path file_path;
    std::wstring entry_point;
private:
    Microsoft::WRL::ComPtr<IDxcBlob> compiled_shader = nullptr;

};

#endif //PALADIN_DXSHADERFILE_H