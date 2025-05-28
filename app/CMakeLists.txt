# CLI应用程序
add_executable(neumann_cli_app cli_main.cpp)
target_link_libraries(neumann_cli_app PRIVATE neumann_core neumann_cli neumann_web)

# Web应用程序
add_executable(neumann_web_app web_main.cpp)
target_link_libraries(neumann_web_app PRIVATE neumann_core neumann_web)

# 安装规则
install(TARGETS neumann_cli_app neumann_web_app
    RUNTIME DESTINATION bin
) 