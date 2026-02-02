//
// Created by James Robertson on 01/02/2026.
//

#ifndef PALADIN_PROFILING_H
#define PALADIN_PROFILING_H
#define TRACY_ENABLE 1
#include <tracy/Tracy.hpp>
#include <tracy/TracyD3D12.hpp>
#ifdef PIX_ENABLE
#include <filesystem>
#include <shlobj_core.h>

#include "Logger.h"

#include "WinPixEventRuntime/pix3.h"
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