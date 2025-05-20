set(CLI_SOURCES
    terminal_ui.cpp
    cli_app.cpp
)

# 创建CLI库
add_library(neumann_cli STATIC ${CLI_SOURCES})

# 添加包含目录
target_include_directories(neumann_cli PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# 设置FTXUI查找选项
set(FTXUI_QUIET ON)
set(FTXUI_BUILD_EXAMPLES OFF)
set(FTXUI_BUILD_DOCS OFF)
set(FTXUI_ENABLE_INSTALL OFF)

# 寻找FTXUI库
find_package(ftxui CONFIG)
if(ftxui_FOUND)
    message(STATUS "使用系统安装的FTXUI库")
else()
    message(FATAL_ERROR "未找到FTXUI库。请先安装FTXUI库或使用vcpkg install ftxui:x64-windows命令安装")
endif()

# 链接依赖库
target_link_libraries(neumann_cli
    PUBLIC
    neumann_core
    ftxui::screen
    ftxui::dom
    ftxui::component
)

# 安装规则
install(TARGETS neumann_cli
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
) 