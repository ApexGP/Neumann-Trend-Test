set(TEST_SOURCES
    test_neumann.cpp
)

# 如果未安装Catch2，则下载
find_package(Catch2 QUIET)
if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.3.2
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# 测试可执行文件
add_executable(neumann_tests ${TEST_SOURCES})

# 链接依赖库
target_link_libraries(neumann_tests
    PRIVATE
    neumann_core
    Catch2::Catch2WithMain
)

# 将测试添加到CTest
include(CTest)
include(Catch)
catch_discover_tests(neumann_tests) 