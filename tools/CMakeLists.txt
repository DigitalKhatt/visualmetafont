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
