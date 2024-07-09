cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(harfbuzz CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source Files
set(SRC_FILES
    harfbuzz/src/hb-unicode.cc    
    harfbuzz/src/hb-aat-layout.cc
    harfbuzz/src/hb-aat-map.cc
    harfbuzz/src/hb-blob.cc
    harfbuzz/src/hb-buffer.cc
    harfbuzz/src/hb-common.cc
    harfbuzz/src/hb-face.cc
    harfbuzz/src/hb-fallback-shape.cc
    harfbuzz/src/hb-font.cc
    harfbuzz/src/hb-ft.cc
    harfbuzz/src/hb-ot-cff1-table.cc
    harfbuzz/src/hb-ot-cff2-table.cc
    harfbuzz/src/hb-ot-face.cc
    harfbuzz/src/hb-ot-font.cc
    harfbuzz/src/hb-ot-layout.cc
    harfbuzz/src/hb-ot-map.cc
    harfbuzz/src/hb-ot-shaper-arabic.cc
    harfbuzz/src/hb-ot-shaper-default.cc
    harfbuzz/src/hb-ot-shaper-hangul.cc
    harfbuzz/src/hb-ot-shaper-hebrew.cc
    harfbuzz/src/hb-ot-shaper-indic-table.cc
    harfbuzz/src/hb-ot-shaper-indic.cc
    harfbuzz/src/hb-ot-shaper-khmer.cc
    harfbuzz/src/hb-ot-shaper-myanmar.cc
    harfbuzz/src/hb-ot-shaper-thai.cc
    #harfbuzz/src/hb-ot-shaper-use-table.cc
    harfbuzz/src/hb-ot-shaper-use.cc
    harfbuzz/src/hb-ot-shaper-syllabic.cc
    harfbuzz/src/hb-ot-shaper-vowel-constraints.cc
    harfbuzz/src/hb-ot-shape-fallback.cc
    harfbuzz/src/hb-ot-shape-normalize.cc
    harfbuzz/src/hb-ot-shape.cc
    harfbuzz/src/hb-ot-tag.cc
    harfbuzz/src/hb-ot-var.cc
    harfbuzz/src/hb-set.cc
    harfbuzz/src/hb-shape-plan.cc
    harfbuzz/src/hb-shape.cc
    harfbuzz/src/hb-shaper.cc        
    harfbuzz/src/hb-unicode.cc    
    harfbuzz/src/hb-static.cc
    harfbuzz/src/hb-ucd.cc
    harfbuzz/src/hb-ot-metrics.cc
    harfbuzz/src/hb-number.cc
    harfbuzz/src/hb-ot-name.cc
    harfbuzz/src/hb-ot-layout-jtst-context.cc
    harfbuzz/src/hb-ot-color.cc
    harfbuzz/src/hb-paint.cc
    harfbuzz/src/hb-paint-extents.cc
    harfbuzz/src/hb-draw.cc
    harfbuzz/src/hb-outline.cc
    harfbuzz/src/hb-buffer-verify.cc
    harfbuzz/src/hb-buffer-serialize.cc
    harfbuzz/src/hb-face-builder.cc
)

# Add library to build.
add_library(${PROJECT_NAME} STATIC ${SRC_FILES} ${HEADERS_FILES})

if (UNIX)
    target_compile_options(${PROJECT_NAME} PUBLIC -fPIC)
    target_link_libraries(${PROJECT_NAME} -static-libgcc -static-libstdc++)
endif (UNIX)

target_include_directories(${PROJECT_NAME} PUBLIC "harfbuzz/src")

if (NOT EMSCRIPTEN)   
  target_compile_definitions(${PROJECT_NAME} PUBLIC HB_MINI)
  #target_compile_definitions(${PROJECT_NAME} PRIVATE HB_NO_JUSTIFICATION)
  if (UNIX)
      target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_PTHREAD)
  endif (UNIX)
else()
  target_compile_options(${PROJECT_NAME} PUBLIC -w)
  target_compile_definitions(${PROJECT_NAME} PUBLIC HB_NO_MT)  
endif()
