cmake_minimum_required(VERSION 3.15)

project(libscratch3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 23)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
pkg_check_modules(ZIP libzip)

file(GLOB PROJ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_compile_options(-Wall -Wextra)
add_compile_options(-fsanitize=leak -fsanitize=address -fsanitize=undefined)
add_library(scratch3 ${PROJ_SRC})
target_include_directories(scratch3 PRIVATE ${JSONCPP_INCLUDE_DIRS} ${ZIP_INCLUDE_DIRS})
target_link_libraries(scratch3 ${JSONCPP_LIBRARIES} ${ZIP_LIBRARIES})
