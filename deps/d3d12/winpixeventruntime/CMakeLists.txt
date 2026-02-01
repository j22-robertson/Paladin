cmake_minimum_required(VERSION 4.0.2)
include(FetchContent)
project(WinPixEventRuntime)

FetchContent_Declare(winpix_fetch URL https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.240308001)
FetchContent_MakeAvailable(winpix_fetch)

add_library(winpix SHARED IMPORTED GLOBAL)
target_include_directories(winpix INTERFACE "${winpix_fetch_SOURCE_DIR}/include/")
set_property(TARGET winpix PROPERTY IMPORTED_IMPLIB   "${winpix_fetch_SOURCE_DIR}/bin/x64/WinPixEventRuntime.lib")
set_property(TARGET winpix PROPERTY IMPORTED_LOCATION "${winpix_fetch_SOURCE_DIR}/bin/x64/WinPixEventRuntime.dll")
set(WINPIX_DIR ${winpix_fetch_SOURCE_DIR} CACHE INTERNAL "")