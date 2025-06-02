# CLI源文件
set(CLI_SOURCES
    data_editor.cpp
    terminal_ui.cpp
    cli_app.cpp
    terminal_utils.cpp
    file_browser.cpp
)

# 创建CLI库
add_library(neumann_cli STATIC ${CLI_SOURCES})

# 添加包含目录
target_include_directories(neumann_cli PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# 链接必需的库（包括FTXUI，因为静态编译）
target_link_libraries(neumann_cli PUBLIC 
    neumann_core 
    neumann_web
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