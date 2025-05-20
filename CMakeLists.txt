cmake_minimum_required(VERSION 3.10)

# 调试信息
message(STATUS "开始CMake配置过程...")

# 添加vcpkg工具链支持
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    message(STATUS "使用vcpkg工具链: $ENV{VCPKG_ROOT}")
    
    # 为MinGW设置正确的triplet
    if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW})
        set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "")
        message(STATUS "设置vcpkg triplet为: ${VCPKG_TARGET_TRIPLET}")
    endif()
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    message(STATUS "使用本地vcpkg工具链")
endif()

project(NeumannTrendTest VERSION 1.0.0 LANGUAGES CXX)

# 平台检测与工具链设置
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "平台: Windows")
    # Windows平台工具链检测
    if(DEFINED ENV{NEUMANN_USE_MINGW} OR MINGW)
        message(STATUS "构建工具链: MinGW")
        # 如果有自定义toolchain文件，则使用该文件配置MinGW
        if(EXISTS "${CMAKE_SOURCE_DIR}/mingw-toolchain.cmake")
            message(STATUS "使用自定义MinGW工具链配置")
            include(${CMAKE_SOURCE_DIR}/mingw-toolchain.cmake)
        endif()
        # 设置MinGW特定的编译选项
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
        
        # MinGW特定的链接选项
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    else()
        message(STATUS "构建工具链: MSVC")
        # MSVC特定的编译选项
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "平台: Linux")
    message(STATUS "构建工具链: GCC")
    # Linux平台使用GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "平台: MacOS")
    message(STATUS "构建工具链: Clang")
    # MacOS平台使用Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
else()
    message(STATUS "平台: 其他")
    message(STATUS "构建工具链: 未知")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# 设置导出编译命令选项（为clangd提供编译数据库）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 输出目录设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 添加子目录
message(STATUS "开始配置核心模块...")
add_subdirectory(src/core)

message(STATUS "开始配置CLI模块...")
add_subdirectory(src/cli)

message(STATUS "开始配置Web模块...")
add_subdirectory(src/web)

message(STATUS "开始配置应用程序...")
add_subdirectory(apps)

message(STATUS "开始配置测试...")
add_subdirectory(tests)

# 安装规则
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/
        DESTINATION bin
        USE_SOURCE_PERMISSIONS) 