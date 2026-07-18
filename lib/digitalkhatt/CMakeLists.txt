cmake_minimum_required(VERSION 3.20)
project(digitalkhatt_core LANGUAGES CXX)

find_package(Clipper2 CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
    pcre2
    GIT_REPOSITORY https://github.com/PCRE2Project/pcre2.git
    GIT_TAG        pcre2-10.47
)

set(PCRE2_BUILD_PCRE2_8   OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2_16  ON  CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2_32  OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2GREP OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(pcre2)

# ---------------------------------------------------------------------------
# digitalkhatt_core — Tajweed / regex library (no Qt, no layout)
# ---------------------------------------------------------------------------

add_library(digitalkhatt_core
  src/Regex16.cpp
  src/tajweed/tajweed_service.cpp
)

target_include_directories(digitalkhatt_core PUBLIC include)
target_compile_features(digitalkhatt_core PUBLIC cxx_std_20)
target_link_libraries(digitalkhatt_core PRIVATE $<BUILD_LOCAL_INTERFACE:pcre2-16-static> Clipper2::Clipper2)


# ---------------------------------------------------------------------------
# digitalkhatt_justify — Qt-free page-justification algorithm
#
# The kashida/alternate-glyph stretching algorithm (ported from
# src/Layout/just_features.cpp). Only depends on HarfBuzz (hb_font_t/hb_buffer_t,
# including the project's custom fork fields) and digitalkhatt_core (PCRE2-16
# regex, Tajweed coloring) — no Qt, no Font/GlyphVis/OtLayout. This is linked
# directly into the main (Qt) app via a small adapter in src/Layout/just_features.cpp,
# and also used by the Qt-free digitalkhatt_layout port below.
# ---------------------------------------------------------------------------

add_library(digitalkhatt_justify
  src/justify/FeatureJustifier.cpp
)

target_include_directories(digitalkhatt_justify PUBLIC include)
target_compile_features(digitalkhatt_justify PUBLIC cxx_std_20)
target_link_libraries(digitalkhatt_justify PUBLIC harfbuzz PRIVATE digitalkhatt_core)


# ---------------------------------------------------------------------------
# digitalkhatt_geometry — Qt-free 2D geometry / GJK-EPA collision library
#
# Polygon/AABB math, cubic-to-polygon tessellation (via polypartition and
# Clipper2), and convex-vs-convex contact queries. Depends on mplib only for
# the mp_graphic_object/mp_fill_object glyph outline structs consumed by
# getGlyphCubic. No Qt.
# ---------------------------------------------------------------------------

add_library(digitalkhatt_geometry
  src/geometry/geometry.cpp
  src/geometry/gjk_epa.cpp
  src/geometry/polypartition.cpp
)

target_include_directories(digitalkhatt_geometry PUBLIC include)
target_compile_features(digitalkhatt_geometry PUBLIC cxx_std_20)
target_link_libraries(digitalkhatt_geometry PUBLIC mplib PRIVATE Clipper2::Clipper2)


# ---------------------------------------------------------------------------
# digitalkhatt_layout — Qt-free XPBD collision-avoidance / mark-positioning
# solver (ported from src/Layout/optimize_layout.cpp). Depends on
# digitalkhatt_geometry (GeometrySet/GJK-EPA contacts) and digitalkhatt_core
# (GlyphLayoutInfo/LineLayoutInfo). No Qt.
# ---------------------------------------------------------------------------

add_library(digitalkhatt_layout
  src/layout/GlyphInstance.cpp
  src/layout/SolverContext.cpp
  src/layout/GapConstraint.cpp
  src/layout/MarkClassifier.cpp
  src/layout/OptimizeLayout.cpp
  src/layout/constraints/YlaneConstraint.cpp
  src/layout/constraints/HorizontalOrderConstraint.cpp
  src/layout/constraints/DotWidthWithinBaseConstraint.cpp
  src/layout/constraints/WaqfAlignXConstraint.cpp
  src/layout/constraints/WaqfPlacementConstraint.cpp
  src/layout/constraints/HardStayAboveConstraint.cpp
  src/layout/constraints/HardStayBelowConstraint.cpp
  src/layout/constraints/MatchMarkPositionWithOffsetConstraint.cpp
  src/layout/constraints/VerticalStackAlignConstraint.cpp
  src/layout/constraints/SqueezeCenterConstraint.cpp
  src/layout/constraints/StackOrderConstraint.cpp
  src/layout/constraints/BaseVicinityConstraint.cpp
  src/layout/constraints/ReturnToAnchorConstraint.cpp
  src/layout/constraints/BowlClusterConstraint.cpp
)

target_include_directories(digitalkhatt_layout PUBLIC include)
target_compile_features(digitalkhatt_layout PUBLIC cxx_std_20)
target_link_libraries(digitalkhatt_layout PUBLIC digitalkhatt_geometry digitalkhatt_core)


# ---------------------------------------------------------------------------
# digitalkhatt_otlayout — Qt-free OpenType layout and font generation engine
# ---------------------------------------------------------------------------

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

include(FetchContent)
FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG v7.5.0
  GIT_SHALLOW TRUE
  FIND_PACKAGE_ARGS 7.5.0
)
FetchContent_MakeAvailable(glaze)

set(OTLAYOUT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/otlayout)
set(OTLAYOUT_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/otlayout/FeaParser)
file(MAKE_DIRECTORY ${OTLAYOUT_GENERATED_DIR})

BISON_TARGET(OtLayoutFeaParser
  ${OTLAYOUT_ROOT}/FeaParser/parser.y
  ${OTLAYOUT_GENERATED_DIR}/parser.cpp
  VERBOSE
)
FLEX_TARGET(OtLayoutFeaScanner
  ${OTLAYOUT_ROOT}/FeaParser/scanner.l
  ${OTLAYOUT_GENERATED_DIR}/scanner.cpp
)
ADD_FLEX_BISON_DEPENDENCY(OtLayoutFeaScanner OtLayoutFeaParser)

add_library(digitalkhatt_otlayout SHARED
  src/otlayout/automedina/automedina.cpp
  src/otlayout/FeaParser/driver.cpp
  src/otlayout/FeaParser/feaast.cpp
  src/otlayout/FeaParser/just.cpp
  src/otlayout/FeaParser/fsm.cpp
  ${BISON_OtLayoutFeaParser_OUTPUTS}
  ${FLEX_OtLayoutFeaScanner_OUTPUTS}
  src/otlayout/qurantext/quran.cpp
  src/otlayout/qurantext/qurancomplex.cpp
  src/otlayout/qurantext/surahs.cpp
  src/otlayout/Layout/JustificationContext.cpp
  src/otlayout/Layout/GlyphVis.cpp
  src/otlayout/Layout/Lookup.cpp
  src/otlayout/Layout/OtLayout.cpp
  src/otlayout/Layout/Subtable.cpp
  src/otlayout/Layout/to_opentype.cpp
  src/otlayout/Layout/FSMDriver.cpp
  src/otlayout/Layout/just_features.cpp
  src/otlayout/Layout/gllobal_strings.cpp
)

target_include_directories(digitalkhatt_otlayout PUBLIC
  ${OTLAYOUT_ROOT}
  ${OTLAYOUT_ROOT}/Layout
  ${OTLAYOUT_ROOT}/FeaParser
  ${OTLAYOUT_ROOT}/automedina
  ${OTLAYOUT_GENERATED_DIR}
)
target_include_directories(digitalkhatt_otlayout PRIVATE ${FLEX_INCLUDE_DIRS})
target_compile_features(digitalkhatt_otlayout PUBLIC cxx_std_20)
target_compile_options(digitalkhatt_otlayout PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)
target_compile_definitions(digitalkhatt_otlayout PRIVATE
  HB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR
)
target_link_libraries(digitalkhatt_otlayout PUBLIC
  harfbuzz
  mplib
  digitalkhatt_core
  digitalkhatt_justify
  digitalkhatt_geometry
  digitalkhatt_layout
  glaze::glaze
)
set_target_properties(digitalkhatt_otlayout PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)
