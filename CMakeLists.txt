cmake_minimum_required(VERSION 3.20)
project(OpenAnimationReplacer-RaySense)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "" FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(CommonLibSSE CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/API/*.cpp"
    "src/API/*.h"
)

add_library(${PROJECT_NAME} MODULE ${SOURCES})

target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)

set_target_properties(${PROJECT_NAME} PROPERTIES 
    OUTPUT_NAME "OpenAnimationReplacer-RaySense"
    PREFIX "" 
    SUFFIX ".dll"
)

# Post-build
set(TEST_PLUGINS_DIR "f:/TuLED/File Mod Skyrim SE/overwrite/SKSE/Plugins")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${TEST_PLUGINS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${TEST_PLUGINS_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Copying DLL to test environment..."
)
