set(CLI_SOURCES
    terminal_ui.cpp
    cli_app.cpp
    terminal_utils.cpp
)

# 创建CLI库
add_library(neumann_cli STATIC ${CLI_SOURCES})

# 添加包含目录
target_include_directories(neumann_cli PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# 链接核心库和Web库
target_link_libraries(neumann_cli PUBLIC neumann_core neumann_web)

# 设置FTXUI查找选项
set(FTXUI_QUIET ON)
set(FTXUI_BUILD_EXAMPLES OFF)
set(FTXUI_BUILD_DOCS OFF)
set(FTXUI_ENABLE_INSTALL OFF)

message(STATUS "开始查找FTXUI库...")
set(FTXUI_FOUND FALSE)

# 使用不同的方法查找FTXUI库
if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW} OR NEUMANN_USE_MINGW)
    message(STATUS "使用MinGW配置，搜索x64-mingw-static triplet下的FTXUI库")
    
    # 直接查找MinGW静态库文件
    if(DEFINED VCPKG_ROOT)
        set(MINGW_LIB_PATH "${VCPKG_ROOT}/installed/x64-mingw-static/lib")
    elseif(DEFINED ENV{VCPKG_ROOT})
        set(MINGW_LIB_PATH "$ENV{VCPKG_ROOT}/installed/x64-mingw-static/lib")
    else()
        message(STATUS "未设置VCPKG_ROOT，尝试在系统路径查找")
        set(MINGW_LIB_PATH "/usr/lib;/usr/local/lib")
    endif()
    
    # 查找头文件目录
    if(DEFINED VCPKG_ROOT)
        set(MINGW_INCLUDE_PATH "${VCPKG_ROOT}/installed/x64-mingw-static/include")
    elseif(DEFINED ENV{VCPKG_ROOT})
        set(MINGW_INCLUDE_PATH "$ENV{VCPKG_ROOT}/installed/x64-mingw-static/include")
    else()
        set(MINGW_INCLUDE_PATH "/usr/include;/usr/local/include")
    endif()
    
    # 显示搜索路径
    message(STATUS "静态库搜索路径: ${MINGW_LIB_PATH}")
    message(STATUS "头文件搜索路径: ${MINGW_INCLUDE_PATH}")
    
    # 查找头文件
    if(EXISTS "${MINGW_INCLUDE_PATH}/ftxui/component/component.hpp")
        message(STATUS "找到FTXUI头文件")
        target_include_directories(neumann_cli PUBLIC ${MINGW_INCLUDE_PATH})
        
        # 查找静态库文件
        find_library(FTXUI_COMPONENT_LIB 
            NAMES libftxui-component.a ftxui-component ftxui_component
            PATHS ${MINGW_LIB_PATH}
            NO_DEFAULT_PATH
        )
        
        find_library(FTXUI_DOM_LIB 
            NAMES libftxui-dom.a ftxui-dom ftxui_dom
            PATHS ${MINGW_LIB_PATH}
            NO_DEFAULT_PATH
        )
        
        find_library(FTXUI_SCREEN_LIB 
            NAMES libftxui-screen.a ftxui-screen ftxui_screen
            PATHS ${MINGW_LIB_PATH}
            NO_DEFAULT_PATH
        )

        message(STATUS "FTXUI_COMPONENT_LIB = ${FTXUI_COMPONENT_LIB}")
        message(STATUS "FTXUI_DOM_LIB = ${FTXUI_DOM_LIB}")
        message(STATUS "FTXUI_SCREEN_LIB = ${FTXUI_SCREEN_LIB}")
        
        if(FTXUI_COMPONENT_LIB AND FTXUI_DOM_LIB AND FTXUI_SCREEN_LIB)
            message(STATUS "找到所有FTXUI静态库")
            target_link_libraries(neumann_cli PUBLIC
                ${FTXUI_COMPONENT_LIB}
                ${FTXUI_DOM_LIB}
                ${FTXUI_SCREEN_LIB}
                neumann_core
                neumann_web
            )
            set(FTXUI_FOUND TRUE)
        endif()
    endif()
    
    # 如果上述方法失败，尝试FindPackage
    if(NOT FTXUI_FOUND)
        message(STATUS "未通过静态库查找找到FTXUI，尝试使用find_package")
        find_package(ftxui CONFIG QUIET)
        if(ftxui_FOUND)
            message(STATUS "使用find_package找到FTXUI")
            target_link_libraries(neumann_cli PUBLIC
                neumann_core
                neumann_web
                ftxui::screen
                ftxui::dom
                ftxui::component
            )
            set(FTXUI_FOUND TRUE)
        else()
            message(STATUS "使用find_package未找到FTXUI")
        endif()
    endif()
else()
    # 对于MSVC，首先尝试标准查找
    find_package(ftxui CONFIG QUIET)
    if(ftxui_FOUND)
        message(STATUS "找到FTXUI库(标准查找)")
        target_link_libraries(neumann_cli PUBLIC
            neumann_core
            neumann_web
            ftxui::screen
            ftxui::dom
            ftxui::component
        )
        set(FTXUI_FOUND TRUE)
    else()
        # 如果标准查找失败，尝试在vcpkg安装路径查找
        if(DEFINED VCPKG_ROOT)
            set(VCPKG_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")
            set(VCPKG_LIB_DIR "${VCPKG_ROOT}/installed/x64-windows/lib")
        elseif(DEFINED ENV{VCPKG_ROOT})
            set(VCPKG_INCLUDE_DIR "$ENV{VCPKG_ROOT}/installed/x64-windows/include")
            set(VCPKG_LIB_DIR "$ENV{VCPKG_ROOT}/installed/x64-windows/lib")
        endif()
        
        if(EXISTS "${VCPKG_INCLUDE_DIR}/ftxui/component/component.hpp")
            message(STATUS "在vcpkg安装路径找到FTXUI头文件")
            target_include_directories(neumann_cli PUBLIC ${VCPKG_INCLUDE_DIR})
            
            find_library(FTXUI_COMPONENT_LIB ftxui-component PATHS ${VCPKG_LIB_DIR})
            find_library(FTXUI_DOM_LIB ftxui-dom PATHS ${VCPKG_LIB_DIR})
            find_library(FTXUI_SCREEN_LIB ftxui-screen PATHS ${VCPKG_LIB_DIR})
            
            if(FTXUI_COMPONENT_LIB AND FTXUI_DOM_LIB AND FTXUI_SCREEN_LIB)
                message(STATUS "找到所有FTXUI库文件")
                target_link_libraries(neumann_cli PUBLIC
                    ${FTXUI_COMPONENT_LIB}
                    ${FTXUI_DOM_LIB}
                    ${FTXUI_SCREEN_LIB}
                    neumann_core
                    neumann_web
                )
                set(FTXUI_FOUND TRUE)
            endif()
        endif()
    endif()
endif()

# 检查是否成功找到FTXUI
if(NOT FTXUI_FOUND)
    # 最后一次尝试，查找所有可能的FTXUI文件，用于调试
    file(GLOB_RECURSE FTXUI_FILES 
        "$ENV{VCPKG_ROOT}/installed/*ftxui*"
    )
    
    if(FTXUI_FILES)
        message(STATUS "找到以下FTXUI相关文件，但无法正确链接:")
        foreach(FILE ${FTXUI_FILES})
            message(STATUS "  ${FILE}")
        endforeach()
    endif()
    
    message(FATAL_ERROR "
    ======================= 错误：未找到FTXUI库 =======================
    未能找到FTXUI库。请使用以下方法之一安装:
    
    Windows (vcpkg):
      vcpkg install ftxui:x64-windows (使用MSVC构建时)
      vcpkg install ftxui:x64-mingw-static (使用MinGW构建时)
      
    Linux:
      手动构建: 参见README.md中的说明
    
    调试信息:
    - VCPKG_ROOT=$ENV{VCPKG_ROOT}
    - VCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
    - CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}
    - MINGW=${MINGW}
    - NEUMANN_USE_MINGW=${NEUMANN_USE_MINGW}
    
    参考README.md了解详细安装步骤。
    =====================================================================")
endif()

# 安装规则
install(TARGETS neumann_cli
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
) 