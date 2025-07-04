# 设置静默模式，抑制一些警告信息
set(BUILDING_SDK "yes" CACHE INTERNAL "")

# 基本配置
if (NOT PROJ)
    message(FATAL_ERROR "必须使用 -DPROJ=<project_name> 指定项目名称")
endif ()

set(CMAKE_C_FLAGS "-std=c99")

# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.12)

# 包含公共配置文件
include(./cmake/common.cmake)

# 定义项目名称和支持的语言
project(${PROJ} C CXX ASM)

# 包含头文件目录
include(./cmake/macros.internal.cmake)
header_directories(${SDK_ROOT}/lib)
header_directories(src/${PROJ})
header_directories(kendryte-standalone-demo/${PROJ})

# 添加库
add_subdirectory(lib)

# 添加源文件
add_source_files(src/${PROJ}/*.c src/${PROJ}/*.s src/${PROJ}/*.S src/${PROJ}/*.cpp)
add_source_files(kendryte-standalone-demo/${PROJ}/*.c kendryte-standalone-demo/${PROJ}/*.s kendryte-standalone-demo/${PROJ}/*.S src/${PROJ}/*.cpp)

# 包含可执行文件配置
include(./cmake/executable.cmake)
