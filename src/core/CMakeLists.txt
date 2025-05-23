set(CORE_SOURCES
    standard_values.cpp
    neumann_calculator.cpp
    data_manager.cpp
    i18n.cpp
    config.cpp
    error_handler.cpp
    excel_reader.cpp
)

# 创建核心库
add_library(neumann_core STATIC ${CORE_SOURCES})

# 添加包含目录
target_include_directories(neumann_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# 查找JSON库
message(STATUS "开始查找JSON库...")
set(JSON_FOUND FALSE)

# 1. 尝试标准查找
find_package(nlohmann_json CONFIG QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "找到nlohmann_json库(标准查找)")
    target_link_libraries(neumann_core PUBLIC nlohmann_json::nlohmann_json)
    set(JSON_FOUND TRUE)
else()
    # 2. 尝试替代名称查找
    find_package(nlohmann-json CONFIG QUIET)
    if(nlohmann-json_FOUND)
        message(STATUS "找到nlohmann-json库(替代名称)")
        target_link_libraries(neumann_core PUBLIC nlohmann_json::nlohmann_json)
        set(JSON_FOUND TRUE)
    else()
        # 3. 尝试直接查找头文件
        set(JSON_HEADER_PATHS)
        
        # 检查vcpkg路径
        if(DEFINED VCPKG_ROOT)
            if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW} OR NEUMANN_USE_MINGW)
                list(APPEND JSON_HEADER_PATHS "${VCPKG_ROOT}/installed/x64-mingw-static/include/nlohmann/json.hpp")
            else()
                list(APPEND JSON_HEADER_PATHS "${VCPKG_ROOT}/installed/x64-windows/include/nlohmann/json.hpp")
            endif()
        elseif(DEFINED ENV{VCPKG_ROOT})
            if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW} OR NEUMANN_USE_MINGW)
                list(APPEND JSON_HEADER_PATHS "$ENV{VCPKG_ROOT}/installed/x64-mingw-static/include/nlohmann/json.hpp")
            else()
                list(APPEND JSON_HEADER_PATHS "$ENV{VCPKG_ROOT}/installed/x64-windows/include/nlohmann/json.hpp")
            endif()
        endif()
        
        # 添加系统路径
        list(APPEND JSON_HEADER_PATHS 
            "/usr/include/nlohmann/json.hpp"
            "/usr/local/include/nlohmann/json.hpp"
        )
        
        # 查找头文件
        foreach(HEADER_PATH ${JSON_HEADER_PATHS})
            if(EXISTS "${HEADER_PATH}")
                message(STATUS "找到JSON头文件: ${HEADER_PATH}")
                get_filename_component(INCLUDE_DIR "${HEADER_PATH}" DIRECTORY)
                get_filename_component(INCLUDE_DIR "${INCLUDE_DIR}" DIRECTORY)
                target_include_directories(neumann_core PUBLIC "${INCLUDE_DIR}")
                set(JSON_FOUND TRUE)
                break()
            endif()
        endforeach()
    endif()
endif()

# 如果所有尝试都失败，则报错
if(NOT JSON_FOUND)
    message(FATAL_ERROR "
    ======================= 错误：未找到JSON库 =======================
    未能找到nlohmann/json库。请使用以下方法之一安装:
    
    Windows (vcpkg):
      vcpkg install nlohmann-json:x64-windows (使用MSVC构建时)
      vcpkg install nlohmann-json:x64-mingw-static (使用MinGW构建时)
      
    Linux:
      Ubuntu/Debian: sudo apt install nlohmann-json3-dev
      Fedora: sudo dnf install nlohmann-json-devel
      Arch Linux: sudo pacman -S nlohmann-json
    
    参考README.md了解详细安装步骤。
    =====================================================================")
endif()

# 安装规则
install(TARGETS neumann_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
) 