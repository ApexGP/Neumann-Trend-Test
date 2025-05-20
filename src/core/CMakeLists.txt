set(CORE_SOURCES
    standard_values.cpp
    neumann_calculator.cpp
    data_manager.cpp
)

# 创建核心库
add_library(neumann_core STATIC ${CORE_SOURCES})

# 添加包含目录
target_include_directories(neumann_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# 查找和链接JSON库
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    # 如果系统中未找到，使用FetchContent下载
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# 链接依赖库
target_link_libraries(neumann_core
    PUBLIC
    nlohmann_json::nlohmann_json
)

# 安装规则
install(TARGETS neumann_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
) 