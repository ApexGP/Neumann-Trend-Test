# 设置系统名称
set(CMAKE_SYSTEM_NAME Windows)

# 定义MinGW路径变量
set(MINGW_ROOT "")

# 打印检测信息
message(STATUS "开始检测MinGW路径...")

# 首先检查环境变量PATH中是否存在gcc/g++
find_program(GCC_PATH gcc)
find_program(GPP_PATH g++)

if(GCC_PATH AND GPP_PATH)
    get_filename_component(MINGW_BIN_PATH "${GCC_PATH}" DIRECTORY)
    get_filename_component(MINGW_ROOT "${MINGW_BIN_PATH}" DIRECTORY)
    message(STATUS "从PATH环境变量找到MinGW: ${MINGW_ROOT}")
    message(STATUS "gcc路径: ${GCC_PATH}")
    message(STATUS "g++路径: ${GPP_PATH}")
# 然后检查系统环境变量中的MinGW路径
elseif(DEFINED ENV{MINGW_ROOT})
    file(TO_CMAKE_PATH "$ENV{MINGW_ROOT}" MINGW_ROOT)
    message(STATUS "使用环境变量中的MinGW: ${MINGW_ROOT}")
# 然后检查常见的MinGW安装位置
else()
    # 检查常见的安装位置
    set(COMMON_MINGW_PATHS
        "C:/MinGW"
        "C:/msys64/mingw64"
        "C:/Program Files/mingw-w64"
        "C:/Users/Domin/scoop/apps/mingw/current"
        "C:/Users/Domin/scoop/apps/gcc/current"
    )
    
    message(STATUS "检查常见MinGW安装路径...")
    foreach(MINGW_PATH ${COMMON_MINGW_PATHS})
        message(STATUS "检查路径: ${MINGW_PATH}")
        if(EXISTS "${MINGW_PATH}")
            set(MINGW_ROOT "${MINGW_PATH}")
            message(STATUS "找到MinGW: ${MINGW_ROOT}")
            break()
        endif()
    endforeach()
endif()

# 如果依然未找到，则使用默认路径
if(MINGW_ROOT STREQUAL "")
    message(WARNING "未找到MinGW路径，尝试使用系统默认设置")
    # 尝试直接使用gcc/g++命令
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
else()
    # 如果找到了MinGW路径，则配置编译器和环境
    message(STATUS "使用MinGW路径: ${MINGW_ROOT}")
    
    # 设置编译器路径
    set(MINGW_BIN "${MINGW_ROOT}/bin")
    
    # 确认bin目录是否存在
    if(EXISTS "${MINGW_BIN}")
        message(STATUS "MinGW bin目录: ${MINGW_BIN}")
        set(CMAKE_C_COMPILER "${MINGW_BIN}/gcc.exe")
        set(CMAKE_CXX_COMPILER "${MINGW_BIN}/g++.exe")
        
        # 设置查找路径
        set(CMAKE_FIND_ROOT_PATH "${MINGW_ROOT}")
        set(CMAKE_PREFIX_PATH "${MINGW_ROOT}")
        
        # 设置环境变量，使clangd能找到正确的头文件
        set(ENV{PATH} "${MINGW_BIN};$ENV{PATH}")
        set(ENV{CPATH} "${MINGW_ROOT}/include;$ENV{CPATH}")
        
        # 设置链接器参数
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    else()
        message(WARNING "MinGW bin目录不存在: ${MINGW_BIN}")
        # 使用系统默认编译器
        set(CMAKE_C_COMPILER "gcc")
        set(CMAKE_CXX_COMPILER "g++")
    endif()
endif()

# 其他设置
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 设置导出编译命令选项（为clangd提供编译数据库）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 