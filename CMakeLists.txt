cmake_minimum_required(VERSION 3.20)
project(OpenAnimationReplacer-RaySense)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# 1. CMAKE 설정 및 옵션 강제:
option(CommonLibSSE_POSH_GENERATION "Generate .posh.d.ts type definition file" OFF)
option(CommonLibSSE_TESTING "Build tests" OFF)
set(CommonLibSSE_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 2. 패키지 로드 (순서 중요):
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")

# 3. 소스 FetchContent: CharmedBaryon/CommonLibSSE-NG.git (main 브랜치)
include(FetchContent)
FetchContent_Declare(CommonLibSSE 
    GIT_REPOSITORY https://github.com/CharmedBaryon/CommonLibSSE-NG.git 
    GIT_TAG main)
FetchContent_MakeAvailable(CommonLibSSE)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/API/*.cpp"
    "src/API/*.h"
)
# UI API requires imgui which is not a dependency - exclude it
list(FILTER SOURCES EXCLUDE REGEX ".*OpenAnimationReplacerAPI-UI.*")

add_library(${PROJECT_NAME} MODULE ${SOURCES})

target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_link_libraries(${PROJECT_NAME} PRIVATE
    CommonLibSSE::CommonLibSSE
    imgui::imgui
    d3d11
    dxgi
)
target_include_directories(${PROJECT_NAME} PRIVATE ${SIMPLEINI_INCLUDE_DIRS})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    OUTPUT_NAME "OpenAnimationReplacer-RaySense"
    PREFIX "" 
    SUFFIX ".dll"
)

# Post-build
set(TEST_PLUGINS_DIR "H:/Game/Lorerim/overwrite/skse/plugins")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${TEST_PLUGINS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${TEST_PLUGINS_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Copying DLL to test environment..."
)
