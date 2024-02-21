cmake_minimum_required(VERSION 3.15)

project(libscratch3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 23)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED)
find_package(SDL2 REQUIRED)
pkg_check_modules(ZIP libzip REQUIRED)

file(GLOB PROJ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_compile_options(-Wall)
if(!${MSVC})
    add_compile_options(-Wextra)
endif()
add_compile_options(-fsanitize=leak -fsanitize=address -fsanitize=undefined)
add_link_options(-fsanitize=leak -fsanitize=address -fsanitize=undefined)

add_library(scratch3 ${PROJ_SRC})
target_include_directories(scratch3 PRIVATE ${BOOST_INCLUDE_DIRS} ${ZIP_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})

target_link_libraries(scratch3 ${BOOST_LIBRARIES} ${ZIP_LIBRARIES} ${SDL2_LIBRARIES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
