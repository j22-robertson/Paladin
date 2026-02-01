cmake_minimum_required(VERSION 4.0.2)
project(Paladin)

set(CMAKE_CXX_STANDARD 20)


add_subdirectory(paladin-core)
add_library(PaladinCore STATIC ${PALADIN_CORE_SOURCES})
target_include_directories(PaladinCore PUBLIC paladin-core/)

add_subdirectory(paladin)
add_executable(PaladinRenderer ${PALADIN_SOURCES})


add_subdirectory(deps/d3d12)
add_subdirectory(deps/glfw)
add_subdirectory(deps/tracy)
add_subdirectory(deps/imgui)

target_link_libraries(imgui PUBLIC glfw)

target_link_libraries(PaladinRenderer PRIVATE
        PaladinCore
        agilitysdk # for latest D3D12 headers & DLLs
        agilitysdk_d3dx12 # d3dx12 helper library from the agility sdk
        d3d12 # core library, assumed to be agility sdk compatible
        dxgi # surface setup etc.
        dxguid # GUIDs for relevant directx types
        dxc # for HLSL shader compilation
        D3D12MemoryAllocator # vram memory allocation
        glfw # Windowing and input
        winpix # Runtime GPU Capture and CPU capture through PIX
        TracyClient # Additional runtime profiling capabilities
        imgui
)

target_compile_definitions(PaladinRenderer PRIVATE PALADIN_ROOT_DIR="${PROJECT_SOURCE_DIR}")

target_compile_definitions(PaladinRenderer PRIVATE PALADIN_AGILITY_SDK_VERSION=${AGILITYSDK_VERSION_MAJOR})
add_custom_command(TARGET PaladinRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PaladinRenderer>/D3D12/"
        COMMAND ${CMAKE_COMMAND} -E copy        -t "$<TARGET_FILE_DIR:PaladinRenderer>/D3D12/" ${AGILITYSDK_D3D12CORE_DLL}
        COMMAND ${CMAKE_COMMAND} -E copy        -t "$<TARGET_FILE_DIR:PaladinRenderer>/D3D12/" ${AGILITYSDK_D3D12SDKLAYERS_DLL}
        COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET PaladinRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:PaladinRenderer> $<TARGET_RUNTIME_DLLS:PaladinRenderer>
        COMMAND_EXPAND_LISTS
)

option(ENABLE_PROFILING "Enable PIX and Tracy" OFF)
if(ENABLE_PROFILING)
    target_compile_definitions(PaladinRenderer PRIVATE
    PIX_ENABLE
    TRACY_ENABLE)
endif ()