include(FetchContent)
FetchContent_Declare (
        stb_image
        URL https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h
        DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(stb_image)

add_library(stb_interface INTERFACE)
target_include_directories(stb_interface INTERFACE ${stb_image_SOURCE_DIR})
