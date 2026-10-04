cmake_minimum_required(VERSION 3.20)

find_library(DIGITALKHATT_TOOLS_SQLITE3_LIBRARY NAMES sqlite3 REQUIRED)

add_executable(digitalkhatt_compare_extreme_lines
  compare_extreme_lines.cpp
)
target_compile_features(digitalkhatt_compare_extreme_lines PRIVATE cxx_std_20)
target_link_libraries(digitalkhatt_compare_extreme_lines PRIVATE
  digitalkhatt::otlayout
  PDFHummus::PDFWriter
  ${DIGITALKHATT_TOOLS_SQLITE3_LIBRARY}
)
target_compile_definitions(digitalkhatt_compare_extreme_lines PRIVATE
  HB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR
  DIGITALKHATT_METAFONT_RESOURCES="${CMAKE_CURRENT_SOURCE_DIR}/../src/Resources/metafont"
  DIGITALKHATT_QURAN_DATABASE="${CMAKE_CURRENT_SOURCE_DIR}/../src/Resources/quran-data.sqlite"
)

# Headless replay of exported placements against live MetaPost outlines.
find_package(Qt5 COMPONENTS Core Gui Svg REQUIRED)
add_executable(digitalkhatt_compare_layout_rendering compare_layout_rendering.cpp)
target_compile_features(digitalkhatt_compare_layout_rendering PRIVATE cxx_std_20)
target_link_libraries(digitalkhatt_compare_layout_rendering PRIVATE
  digitalkhatt::otlayout Qt5::Core Qt5::Gui Qt5::Svg
)
target_compile_definitions(digitalkhatt_compare_layout_rendering PRIVATE
  QT_NO_KEYWORDS
  DIGITALKHATT_METAFONT_RESOURCES="${CMAKE_CURRENT_SOURCE_DIR}/../src/Resources/metafont"
)
if(BUILD_TESTING)
  add_test(NAME digitalkhatt.layout_rendering_measurements
    COMMAND digitalkhatt_compare_layout_rendering --self-test)
endif()
