cmake_minimum_required(VERSION 3.15)

project(libscratch3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 23)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS json)
pkg_check_modules(ZIP libzip REQUIRED)
pkg_check_modules(SDL2 sdl2 REQUIRED)
pkg_check_modules(SDL2_IMAGE SDL2_image REQUIRED)
pkg_check_modules(SDL2_MIXER SDL2_mixer REQUIRED)

file(GLOB PROJ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


if(${MSVC})
add_compile_options(-Wall)
else()
add_compile_options(-Wall -Wextra -Wpedantic)
endif()
add_compile_options(-fsanitize=thread -fsanitize=undefined)
add_link_options(-fsanitize=thread -fsanitize=undefined -Wl,--add-needed)

add_library(scratch3 SHARED ${PROJ_SRC})
target_include_directories(scratch3 PRIVATE Boost::boost ${ZIP_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})

target_link_libraries(scratch3 Boost::json ${ZIP_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

include(CTest)
add_subdirectory(test)
enable_testing()
