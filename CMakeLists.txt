cmake_minimum_required(VERSION 3.2)

project(mymuduo)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_CFLAGS ${CMAKE_CXX_CFLAGS} -g)

# 设置项目头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 设置项目库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 设置项目可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置项目库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
add_subdirectory(examples)