//
// Created by James Robertson on 01/02/2026.
//

#ifndef PALADIN_PROFILING_H
#define PALADIN_PROFILING_H
#include <tracy/Tracy.hpp>
#include <tracy/TracyD3D12.hpp>

#include "WinPixEventRuntime/pix3.h"
namespace ProfileColors {
    static constexpr unsigned int Red    = 0xFFFF0000;
    static constexpr unsigned int Green  = 0xFF00FF00;
    static constexpr unsigned int Blue   = 0xFF0000FF;
}
/// PIX COLORS FOR SCOPED EVENTS AND MARKERS
 const UINT32 frame_color = PIX_COLOR(64,255,0);
#ifdef TRACY_ENABLE
// Tracy
#define PALADIN_SCOPED_CPU_PROFILE(name, color)\
    ZoneScoped;\
    ZoneColor(color);\
    ZoneName(name, strlen(name))
// Tracy
#define PALADIN_SCOPED_GPU_PROFILE(ctx, command_list, name) TracyD3D12Zone(ctx, command_list, name);

// Tracy
#define PALADIN_SCOPED_GPU_PROFILE_C(ctx, command_list, name, color) TracyD3D12ZoneC(ctx, command_list, name, color);
// Not used by Tracy
#define PALADIN_BEGIN_GPU_PROFILE(command_queue,color,name)
#elif PIX_ENABLE
// Pix
#define PALADIN_SCOPED_CPU_PROFILE(name, color) PIXScopedEvent(color,name)
// Pix
#define PALADIN_SCOPED_GPU_PROFILE(ctx, command_list, name) PIXScopedEvent(command_list, name)
// Pix
#define PALADIN_SCOPED_GPU_PROFILE_C(ctx, command_list, name, color) PIXScopedEvent(command_list, color, name);
// Pix
#define PALADIN_BEGIN_GPU_PROFILE(command_queue,color,name) PIXScopedEvent(command_queue, frame_color, name);
#else
#define PALADIN_SCOPED_CPU_PROFILE(name,color)
#define PALADIN_SCOPED_GPU_PROFILE(ctx, command_list, name)
#define PALADIN_SCOPED_GPU_PROFILE_C(ctx, command_list, name, color);
#define PALADIN_BEGIN_GPU_PROFILE(command_queue,color,name)
#endif





#ifdef PIX_ENABLE
#include <filesystem>
#include <shlobj_core.h>

#include "Logger.h"

static std::wstring GetLatestWinPixGpuCapturerPath_Cpp17()
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    std::filesystem::path pixInstallationPath = programFilesPath;
    pixInstallationPath /= "Microsoft PIX";

    std::wstring newestVersionFound;

    for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
    {
        if (directory_entry.is_directory())
        {
            if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
            {
                newestVersionFound = directory_entry.path().filename().c_str();
            }
        }
    }

    if (newestVersionFound.empty())
    {
        return L"No pix installation found";
    }

    return pixInstallationPath/ newestVersionFound / L"WinPixGpuCapturer.dll";
}

static void LoadWinPixEventRuntime()
{
    if (GetModuleHandle(reinterpret_cast<LPCSTR>(L"WinPixGpuCapturer.dll")) == 0)
    {
        std::wstring pix_path = GetLatestWinPixGpuCapturerPath_Cpp17();
        PALADIN_LOG(INFO, "Loading PIX from: " + ConvertWString(pix_path))
        LoadLibraryW(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
    }
}
#endif

#endif //PALADIN_PROFILING_H