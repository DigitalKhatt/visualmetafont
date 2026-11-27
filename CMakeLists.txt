cmake_minimum_required(VERSION 3.24)

project(digitalkhatt)
option(DIGITALKHATT_BUILD_GUI "Build the VisualMetaFont Qt editor" ON)
if(NOT DEFINED DIGITALKHATT_BUILD_QT_TOOLS)
  set(DIGITALKHATT_BUILD_QT_TOOLS "${DIGITALKHATT_BUILD_GUI}")
endif()
include(cmake/EngineSource.cmake)
include(CTest)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_options($<$<C_COMPILER_ID:MSVC>:/utf-8>)
#add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
if (STATICMSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if (NOT EMSCRIPTEN)
  include(FetchContent)
  FetchContent_Declare(
    PDFHummus
    GIT_REPOSITORY https://github.com/galkahana/PDF-Writer.git
    GIT_TAG v4.9.0
    FIND_PACKAGE_ARGS
  )
  FetchContent_MakeAvailable(PDFHummus)
endif()

#file(GLOB_RECURSE OTHERS ../madinafont/*.* ../oldmadinafont/*.* ../indopakfont/*.* )

#add_custom_target(files SOURCES ${OTHERS})


add_compile_options($<$<C_COMPILER_ID:MSVC>:/Zc:__cplusplus>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)

set(DIGITALKHATT_QURAN_DATABASE "${CMAKE_CURRENT_SOURCE_DIR}/src/Resources/quran-data.sqlite"
  CACHE FILEPATH "Quran SQLite corpus used by the native Mushaf tools and corpus test")
add_subdirectory("${DIGITALKHATT_ENGINE_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/lib")
get_target_property(DIGITALKHATT_ENGINE_RESOURCES digitalkhatt::otlayout DIGITALKHATT_RESOURCE_DIR)
if (NOT EMSCRIPTEN AND DIGITALKHATT_BUILD_GUI)
  add_subdirectory(lib/QtPropertyBrowser)
  add_subdirectory(src)
endif()
