cmake_minimum_required(VERSION 3.0) 

project(semi-IndexExtractor)

# 这行设置编译生成的库文件输出路径。
#这里定义了库文件的输出路径为 ${PROJECT_SOURCE_DIR}/lib，其中 PROJECT_SOURCE_DIR 是 CMake 内置的变量，指代当前 CMakeLists.txt 文件所在的目录。
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib) 

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -std=c++11 -O3 -g -march=native -pthread -fopenmp") 

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# 这行指定了包含头文件的目录。${PROJECT_SOURCE_DIR}/include 是项目中的 include 目录，里面可能包含公共头文件，供编译器在编译时搜索这些文件。
include_directories(${PROJECT_SOURCE_DIR}/include)

# 这行添加了 libsrc 子目录，表示这个项目中包含一个 libsrc 子模块。
# 这个子模块可能会生成一些库，供其他部分使用。libsrc 目录中应该有另一个 CMakeLists.txt 文件，负责定义该模块的构建。
add_subdirectory(${PROJECT_SOURCE_DIR}/libsrc)

# 这行与上一行类似，添加了 src 子目录。src 子目录可能包含项目的主要代码模块，也应该有自己的 CMakeLists.txt 文件，定义了如何构建源代码并链接库。
add_subdirectory(${PROJECT_SOURCE_DIR}/src)

link_directories(${LIBRARY_OUTPUT_PATH})