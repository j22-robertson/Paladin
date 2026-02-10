cmake_minimum_required(VERSION 4.0.2)
project(Paladin)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    set(CMAKE_OBJECT_PATH_MAX 660)
endif ()
add_subdirectory(paladin-core)
add_library(PaladinCore STATIC ${PALADIN_CORE_SOURCES})
target_include_directories(PaladinCore PUBLIC paladin-core/)

target_compile_definitions(PaladinCore PRIVATE PALADIN_ROOT_DIR="${PROJECT_SOURCE_DIR}")
add_subdirectory(paladin)
add_executable(PaladinRenderer ${PALADIN_SOURCES})


option(ENABLE_TRACY "Enable Tracy" OFF)
option(ENABLE_PIX "Enable PIX" OFF)

if(ENABLE_TRACY AND ENABLE_PIX)
    message(FATAL_ERROR "Conflict: Unable to use PIX and Tracy simultaneously")
endif ()

if(ENABLE_TRACY)
    set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
    target_compile_definitions(PaladinCore PRIVATE
            TRACY_ENABLE
    TRACY_NO_CALLSTACK)
    target_compile_definitions(PaladinRenderer PRIVATE
            TRACY_ENABLE
            TRACY_EXPORTS
            TRACY_NO_CALLSTACK)
else ()
    set(TRACY_ENABLE OFF CACHE BOOL "" FORCE)
endif ()

if(ENABLE_PIX)
    set(TRACY_ENABLE OFF CACHE BOOL "" FORCE)
    target_compile_definitions(PaladinRenderer PRIVATE PIX_ENABLE)
endif ()



add_subdirectory(deps/d3d12)
add_subdirectory(deps/glfw)
add_subdirectory(deps/tracy)
add_subdirectory(deps/imgui)
add_subdirectory(deps/assimp)
add_subdirectory(deps/glm)
add_subdirectory(deps/stb_image)

target_include_directories(PaladinCore INTERFACE ${stb_image_SOURCE_DIR})

set(GLM_FORCE_LEFT_HANDED ON CACHE BOOL "" FORCE)
#https://github.com/g-truc/glm/blob/master/manual.md#section2_16
set(GLM_FORCE_DEPTH_ZERO_TO_ONE ON CACHE BOOL "" FORCE)
#https://github.com/g-truc/glm/blob/master/manual.md#section2_17
set(GLM_FORCE_DEFAULT_ALIGNED_GENTYPES ON CACHE BOOL "" FORCE)

target_compile_definitions(PaladinRenderer PUBLIC GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(imgui PUBLIC glfw)

target_link_libraries(PaladinCore PUBLIC
        assimp
        stb_interface)



target_link_libraries(PaladinRenderer PRIVATE
        PaladinCore # Core library for Paladin
        agilitysdk # for latest D3D12 headers & DLLs
        agilitysdk_d3dx12 # d3dx12 helper library from the agility sdk
        d3d12 # core library, assumed to be agility sdk compatible
        dxgi # surface setup etc.
        dxguid # GUIDs for relevant directx types
        dxc # for HLSL shader compilation
        D3D12MemoryAllocator # vram memory allocation
        glfw # Windowing and input
        winpix # Runtime GPU Capture and CPU capture through PIX
        Tracy::TracyClient # Additional runtime profiling capabilities
        imgui # GUI
        assimp # Mesh loading
        glm::glm
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
option(ENABLE_LOGGING "Enable logging" ON)
if(ENABLE_LOGGING)
    target_compile_definitions(PaladinCore PRIVATE PALADIN_LOG_ENABLE)
    target_compile_definitions(PaladinRenderer PRIVATE PALADIN_LOG_ENABLE)

endif ()

