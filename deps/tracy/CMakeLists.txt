cmake_minimum_required(VERSION 4.0.2)
include(FetchContent)
FetchContent_Declare (
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tracy)
set(TRACY_PROFILER_SRC "${tracy_SOURCE_DIR}/profiler/")
set(TRACY_PROFILER_BUILD "${CMAKE_BINARY_DIR}/tracy-profiler-build")

add_custom_target(tracy_profiler ALL
        COMMAND ${CMAKE_COMMAND} -B "${TRACY_PROFILER_BUILD}" -S "${TRACY_PROFILER_SRC}" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        COMMAND ${CMAKE_COMMAND} --build "${TRACY_PROFILER_BUILD}" --parallel
        COMMENT "Building Tracy Profiler GUI"
)