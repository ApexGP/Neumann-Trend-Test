# CLI应用程序
add_executable(neumann_cli_app cli_main.cpp)
target_link_libraries(neumann_cli_app PRIVATE neumann_core neumann_cli neumann_web)

# Web应用程序
add_executable(neumann_web_app web_main.cpp)
target_link_libraries(neumann_web_app PRIVATE neumann_core neumann_web)

# MinGW静态链接设置
if(MINGW OR DEFINED ENV{NEUMANN_USE_MINGW})
    # 确保完全静态链接
    target_link_options(neumann_cli_app PRIVATE -static -static-libgcc -static-libstdc++ -Wl,-Bstatic -pthread)
    target_link_options(neumann_web_app PRIVATE -static -static-libgcc -static-libstdc++ -Wl,-Bstatic -pthread)
    
    # 添加Windows特定的静态库
    if(WIN32)
        target_link_libraries(neumann_cli_app PRIVATE ws2_32 wsock32 winpthread)
        target_link_libraries(neumann_web_app PRIVATE ws2_32 wsock32 winpthread)
    endif()
endif()

# 安装规则
install(TARGETS neumann_cli_app neumann_web_app
    RUNTIME DESTINATION bin
) 