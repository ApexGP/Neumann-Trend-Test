set(WEB_SOURCES
    web_server.cpp
)

# 创建Web库
add_library(neumann_web STATIC ${WEB_SOURCES})

# 添加包含目录
target_include_directories(neumann_web PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# 设置Crow查找选项
set(CROW_QUIET ON)
set(CROW_BUILD_EXAMPLES OFF)
set(CROW_BUILD_TESTS OFF)

# 查找Crow HTTP库
message(STATUS "开始查找Crow库...")
set(CROW_FOUND FALSE)

# 1. 尝试标准查找
find_package(Crow CONFIG QUIET)
if(Crow_FOUND)
    message(STATUS "找到Crow库(标准查找)")
    target_link_libraries(neumann_web PUBLIC Crow::Crow neumann_core)
    if(WIN32)
        target_link_libraries(neumann_web PUBLIC ws2_32 wsock32)
        # MinGW静态链接设置
        if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW})
            target_link_libraries(neumann_web PUBLIC winpthread)
        endif()
    endif()
    set(CROW_FOUND TRUE)
else()
    # 2. 尝试直接查找头文件
    set(CROW_HEADER_PATHS)
    
    # 检查vcpkg路径
    if(DEFINED VCPKG_ROOT)
        if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW} OR NEUMANN_USE_MINGW)
            list(APPEND CROW_HEADER_PATHS 
                "${VCPKG_ROOT}/installed/x64-mingw-static/include/crow.h"
                "${VCPKG_ROOT}/installed/x64-mingw-static/include/crow/crow.h"
            )
        else()
            list(APPEND CROW_HEADER_PATHS 
                "${VCPKG_ROOT}/installed/x64-windows/include/crow.h"
                "${VCPKG_ROOT}/installed/x64-windows/include/crow/crow.h"
            )
        endif()
    elseif(DEFINED ENV{VCPKG_ROOT})
        if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW} OR NEUMANN_USE_MINGW)
            list(APPEND CROW_HEADER_PATHS 
                "$ENV{VCPKG_ROOT}/installed/x64-mingw-static/include/crow.h"
                "$ENV{VCPKG_ROOT}/installed/x64-mingw-static/include/crow/crow.h"
            )
        else()
            list(APPEND CROW_HEADER_PATHS 
                "$ENV{VCPKG_ROOT}/installed/x64-windows/include/crow.h"
                "$ENV{VCPKG_ROOT}/installed/x64-windows/include/crow/crow.h"
            )
        endif()
    endif()
    
    # 添加系统路径
    list(APPEND CROW_HEADER_PATHS 
        "/usr/include/crow.h"
        "/usr/include/crow/crow.h"
        "/usr/local/include/crow.h"
        "/usr/local/include/crow/crow.h"
    )
    
    # 查找头文件
    foreach(HEADER_PATH ${CROW_HEADER_PATHS})
        if(EXISTS "${HEADER_PATH}")
            message(STATUS "找到Crow头文件: ${HEADER_PATH}")
            
            if(HEADER_PATH MATCHES ".*\\.h$")
                # 如果找到的是crow.h文件，则使用其所在目录
                get_filename_component(INCLUDE_DIR "${HEADER_PATH}" DIRECTORY)
            else()
                # 如果找到的是目录，则使用其父目录
                get_filename_component(INCLUDE_DIR "${HEADER_PATH}" DIRECTORY)
                get_filename_component(INCLUDE_DIR "${INCLUDE_DIR}" DIRECTORY)
            endif()
            
            target_include_directories(neumann_web PUBLIC "${INCLUDE_DIR}")
            
            # Crow是仅头文件库，只需链接核心库
            target_link_libraries(neumann_web PUBLIC neumann_core)
            
            # Windows下需要链接网络库
            if(WIN32)
                target_link_libraries(neumann_web PUBLIC ws2_32 wsock32)
                # MinGW静态链接设置
                if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW})
                    target_link_libraries(neumann_web PUBLIC winpthread)
                endif()
            endif()
            
            set(CROW_FOUND TRUE)
            break()
        endif()
    endforeach()
endif()

# 如果所有尝试都失败，则报错
if(NOT CROW_FOUND)
    message(FATAL_ERROR "
    ======================= 错误：未找到Crow库 =======================
    未能找到Crow HTTP库。请使用以下方法之一安装:
    
    Windows (vcpkg):
      vcpkg install crow:x64-windows (使用MSVC构建时)
      vcpkg install crow:x64-mingw-static (使用MinGW构建时)
      
    Linux:
      手动构建: 参见README.md中的说明
    
    参考README.md了解详细安装步骤。
    =====================================================================")
endif()

# 安装规则
install(TARGETS neumann_web
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装Web资源文件
install(DIRECTORY ${CMAKE_SOURCE_DIR}/web/
    DESTINATION share/neumann/web
    FILES_MATCHING PATTERN "*.html" PATTERN "*.css" PATTERN "*.js"
) 