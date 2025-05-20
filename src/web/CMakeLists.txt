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

# 寻找Crow HTTP库
find_package(Crow CONFIG)
if(Crow_FOUND)
    message(STATUS "使用系统安装的Crow库")
else()
    message(FATAL_ERROR "未找到Crow库。请先安装Crow库或使用vcpkg install crow:x64-windows命令安装")
endif()

# 链接依赖库
target_link_libraries(neumann_web
    PUBLIC
    neumann_core
    Crow::Crow
)

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