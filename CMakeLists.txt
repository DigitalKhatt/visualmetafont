cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(digitalkhatt)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_options($<$<C_COMPILER_ID:MSVC>:/utf-8>)
#add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

file(GLOB OTHERS files/*.mp files/*.fea  files/images/*.jpg)

add_custom_target(files SOURCES ${OTHERS})


add_compile_options($<$<C_COMPILER_ID:MSVC>:/Zc:__cplusplus>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)

add_subdirectory(lib/harfbuzz)
add_subdirectory(lib/mplib)
add_subdirectory(lib/QtPropertyBrowser)
add_subdirectory(src)


