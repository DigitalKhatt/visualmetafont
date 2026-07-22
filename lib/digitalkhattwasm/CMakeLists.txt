cmake_minimum_required(VERSION 3.20)
project(digitalkhattwasm LANGUAGES CXX)

# ---------------------------------------------------------------------------
# VisualMetaFontWasm — Emscripten bindings exposing digitalkhatt_otlayout's
# Quran shaping/justification engine to the web app. The sources
# unconditionally use <emscripten/bind.h>, so this target requires the
# Emscripten toolchain (emcmake/emcc) to configure and build -- there is no
# native fallback. Deliberately NOT EXCLUDE_FROM_ALL: this whole file already
# hard-fails (below) unless EMSCRIPTEN is set, and it's only ever reached via
# an `if(EMSCRIPTEN)`-guarded add_subdirectory() in the first place, so the
# target can never exist in a non-Emscripten configure regardless. Building
# it as part of the default `[all]` target means a plain build (e.g. VS
# Code's CMake Tools without explicitly selecting a build target) still
# produces VisualMetaFontWasm.wasm, not just the sibling madina_font plugin.
#
# Debug config keeps DWARF (-g at link time too, since wasm-ld strips it
# otherwise) so the actual .wasm can be debugged directly: open Chrome
# DevTools with the "C/C++ DevTools Support (DWARF)" extension, or run the
# generated .js under `node --inspect-brk` and attach from chrome://inspect.
# Either way breakpoints/stepping work against this original C++ source.
# ---------------------------------------------------------------------------

if (NOT EMSCRIPTEN)
  message(FATAL_ERROR "digitalkhattwasm requires the Emscripten toolchain -- configure with emcmake/emcc.")
endif()

# mplib/harfbuzz/digitalkhatt_otlayout need C++20 (std::format,
# unordered_map::contains, ...). When nested under visualmetafont's own
# CMakeLists.txt this is already set at that scope; set it here too so a
# standalone configure of just this directory still gets it.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Self-sufficient: pull in its own dependency chain (harfbuzz, mplib,
# digitalkhatt_otlayout) when not already provided by a parent project.
# Normally reached via the root-level "emscripten" CMake preset
# (/Users/amin/projects/digitalkhatt/CMakePresets.json), which skips the
# Qt-dependent parts of the tree (visualmetafont/src, lib/QtPropertyBrowser,
# the font projects) under EMSCRIPTEN -- but this guard means this directory
# can still be configured entirely on its own too, with no parent involved.
if (NOT TARGET digitalkhatt_otlayout)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../harfbuzz ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../mplib ${CMAKE_CURRENT_BINARY_DIR}/mplib)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../digitalkhatt ${CMAKE_CURRENT_BINARY_DIR}/digitalkhatt)
endif()

set(WEBLIB_NAME VisualMetaFontWasm)

set(WEBLIB_FILES
  src/visualmetafontwasm.cpp
  src/quranshaper.h
)

add_executable(${WEBLIB_NAME} ${WEBLIB_FILES})

target_compile_definitions(${WEBLIB_NAME} PRIVATE HB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR COMPILING_DLL)

add_dependencies(${WEBLIB_NAME} digitalkhatt_otlayout)

target_link_libraries(${WEBLIB_NAME} PUBLIC digitalkhatt_otlayout PRIVATE glaze::glaze)

target_compile_options(${WEBLIB_NAME} PUBLIC -w)

target_compile_options(${WEBLIB_NAME} PRIVATE
  # -fsanitize=leak
  "SHELL:-s USE_ZLIB=1"
  "SHELL:-s ALLOW_MEMORY_GROWTH=1"
  # "SHELL:-s USE_FREETYPE=1"
  # "SHELL:-s USE_LIBPNG=1"
  )

option(DIGITALKHATTWASM_LOAD_TIME_LINK
  "Link the font side modules at VisualMetaFontWasm load time instead of loading the selected font at runtime"
  ON)

if(DIGITALKHATTWASM_LOAD_TIME_LINK)
  target_link_options(${WEBLIB_NAME} PRIVATE "SHELL:-s MAIN_MODULE=2")
else()
  target_link_options(${WEBLIB_NAME} PRIVATE "SHELL:-s MAIN_MODULE=1")
endif()

# Emscripten dynamic linking offers two ways to bring a side module in:
#   - run-time: an explicit dlopen() call (OtLayout.cpp does this today) --
#     madina_font.wasm is fetched lazily, only once shaping actually needs it.
#   - load-time: the side module is listed directly on the main module's own
#     link line. wasm-ld then records it as a "needed" dependency in this
#     module's dylink.0 section, and Emscripten's JS glue fetches and links
#     it automatically before the module becomes ready -- eagerly, on every
#     load, whether or not that session ends up needing it. OtLayout.cpp's
#     dlopen() call is unchanged either way: it asks for the same file name
#     (SLPREFIX+stem+SLEXT, e.g. "libmadina.wasm") regardless, and Emscripten
#     resolves that against its own registry of already-loaded libraries --
#     when load-time linking already put it there, dlopen() finds it
#     immediately and does no work (no fetch, no re-link).
# Off by default: the eager-vs-lazy fetch tradeoff depends on whether a given
# deployment's typical session needs madina_font right away (load-time linking
# lets the browser start fetching it in parallel with the rest of startup) or
# only sometimes (run-time dlopen() avoids paying for it when unused).
if(DIGITALKHATTWASM_LOAD_TIME_LINK)
  target_link_libraries(${WEBLIB_NAME} PRIVATE "$<TARGET_FILE:madina_font>")
  target_link_libraries(${WEBLIB_NAME} PRIVATE "$<TARGET_FILE:oldmadina_font>")
  target_link_libraries(${WEBLIB_NAME} PRIVATE "$<TARGET_FILE:indopak_font>")
endif()

target_link_options(${WEBLIB_NAME} PRIVATE --bind --use-preload-plugins  # -fsanitize=leak
  "SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=1"
  #"SHELL:-s WASM=1"
  #"SHELL:-s WASM=0 --separate-asm -s \"SEPARATE_ASM_MODULE_NAME='VMetaFont'\""
  #"SHELL:-s ASSERTIONS=1"
  #"SHELL:-s FULL_ES2=1"
  #"SHELL:-s USE_WEBGL2=1"
  # USE_ES6_IMPORT_META=0 dropped: newer Emscripten forces this on and
  # rejects the flag outright ("Disabling is no longer supported").
  # DISABLE_EXCEPTION_CATCHING=0 (i.e. catching *enabled*) is required:
  # quranshaper.h and MPFont both use try/catch around real throw sites
  # (OtLayout's constructor, MPFont::initialize/execute). Without it,
  # ___cxa_throw aborts at runtime ("exception catching is not enabled").
  "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
  "SHELL:-s NO_EXIT_RUNTIME=0"
  # "SHELL:-s \"BINARYEN_TRAP_MODE='clamp'\""
  "SHELL:-s ALLOW_MEMORY_GROWTH=1" #-O3
  # Old Medina's nested MetaPost derived-glyph expansion needs more than
  # Emscripten's small default C/C++ stack. This stack belongs to the main
  # module and is shared with dynamically loaded font side modules.
  "SHELL:-s STACK_SIZE=1048576"
  "SHELL:-s USE_ZLIB=1"
  #"SHELL:-s USE_SDL=2"
  #"SHELL:-s EXPORT_ES6=1"
  # getExceptionMessage: embind marshals thrown C++ exceptions to JS as an
  # opaque pointer; this decodes it back to the real what() text.
  "SHELL:-s \"EXPORTED_RUNTIME_METHODS=['FS','print','printErr','getExceptionMessage']\""
  #"SHELL:-s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[\\\"$Browser\\\"]"
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s IMPORTED_MEMORY"
  #"SHELL:-s TOTAL_MEMORY=1048576000"
  "SHELL:-s ENVIRONMENT=web"
  #"SHELL:-s \"EXPORTED_FUNCTIONS=['_main']\""
  #--post-js "${CMAKE_CURRENT_SOURCE_DIR}/post-js.js"
  )

# wasm-ld strips DWARF unless -g is also passed at link time.
target_link_options(${WEBLIB_NAME} PRIVATE $<$<CONFIG:Debug>:-g>)

if(WASMTYPE STREQUAL "WASM")

  target_link_options(${WEBLIB_NAME} PRIVATE
    "SHELL:-s WASM=1"
    "SHELL:-s EXPORT_ES6=1"
    # DEMANGLE_SUPPORT=1 dropped: no longer a valid setting on newer
    # Emscripten (demangling is handled unconditionally now).
    )

  add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE_DIR:${WEBLIB_NAME}>/${WEBLIB_NAME}.js" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/app/services/quranservice"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE_DIR:${WEBLIB_NAME}>/${WEBLIB_NAME}.wasm" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
    COMMENT "Copying to output directory")



else()
  target_link_options(${WEBLIB_NAME} PRIVATE
    "SHELL:-s WASM=0"
    "SHELL:-s EXPORT_NAME='VMASM'"
    "SHELL:-s TOTAL_MEMORY=26214400"
    )

  add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE_DIR:${WEBLIB_NAME}>/${WEBLIB_NAME}.js"		"${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/vmasm"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE_DIR:${WEBLIB_NAME}>/${WEBLIB_NAME}.js.mem"	"${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/vmasm"
    COMMENT "Copying to output directory")

endif()

set(MADINAFONT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../madinafont")
set(OLDMADINAFONT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../oldmadinafont")
set(INDOPAKFONT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../indopakfont")
set(METAFONT_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src/Resources/metafont")
set(ANGULAR_WASM_ASSETS_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt-js/apps/site-angular/src/assets/wasm_masahif")

# Keep each font project in its own directory because glyphs.mp,
# features.fea, and parameters.json have identical names. The Angular service
# selects one directory from the /wasm/newmedina, /wasm/oldmedina, or
# /wasm/indopak15 route and mirrors it into MEMFS before constructing
# OtLayoutMushaf.
add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${ANGULAR_WASM_ASSETS_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${ANGULAR_WASM_ASSETS_DIR}/fonts/madina"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${ANGULAR_WASM_ASSETS_DIR}/fonts/oldmadina"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${ANGULAR_WASM_ASSETS_DIR}/fonts/indopak"
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE_DIR:${WEBLIB_NAME}>/${WEBLIB_NAME}.js" "${ANGULAR_WASM_ASSETS_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE_DIR:${WEBLIB_NAME}>/${WEBLIB_NAME}.wasm" "${ANGULAR_WASM_ASSETS_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/mfplain.mp" "${ANGULAR_WASM_ASSETS_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/mpost.mp" "${ANGULAR_WASM_ASSETS_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/vmf.mp" "${ANGULAR_WASM_ASSETS_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/madina.mp" "${MADINAFONT_DIR}/glyphs.mp" "${MADINAFONT_DIR}/features.fea" "${MADINAFONT_DIR}/parameters.json" "${ANGULAR_WASM_ASSETS_DIR}/fonts/madina"
  COMMAND "${CMAKE_COMMAND}" -E copy "${OLDMADINAFONT_DIR}/oldmadina.mp" "${OLDMADINAFONT_DIR}/glyphs.mp" "${OLDMADINAFONT_DIR}/features.fea" "${OLDMADINAFONT_DIR}/parameters.json" "${ANGULAR_WASM_ASSETS_DIR}/fonts/oldmadina"
  COMMAND "${CMAKE_COMMAND}" -E copy "${INDOPAKFONT_DIR}/indopak.mp" "${INDOPAKFONT_DIR}/glyphs.mp" "${INDOPAKFONT_DIR}/features.fea" "${INDOPAKFONT_DIR}/parameters.json" "${ANGULAR_WASM_ASSETS_DIR}/fonts/indopak"
  COMMENT "Copying WASM mushaf runtime and font assets to Angular")


  add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:madina_font>" "${ANGULAR_WASM_ASSETS_DIR}/fonts/madina"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:oldmadina_font>" "${ANGULAR_WASM_ASSETS_DIR}/fonts/oldmadina"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:indopak_font>" "${ANGULAR_WASM_ASSETS_DIR}/fonts/indopak"
    COMMENT "Copying WASM font side modules to Angular")

if(NOT DIGITALKHATTWASM_STANDALONE)
  add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:madina_font>" "${ANGULAR_WASM_ASSETS_DIR}/fonts/madina"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:oldmadina_font>" "${ANGULAR_WASM_ASSETS_DIR}/fonts/oldmadina"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:indopak_font>" "${ANGULAR_WASM_ASSETS_DIR}/fonts/indopak"
    COMMENT "Copying WASM font side modules to Angular")
else()
  # A standalone digitalkhattwasm configure does not build the font targets;
  # use the side modules already produced in each font project.
  add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/libmadina.wasm" "${ANGULAR_WASM_ASSETS_DIR}/fonts/madina"
    COMMAND "${CMAKE_COMMAND}" -E copy "${OLDMADINAFONT_DIR}/liboldmadina.wasm" "${ANGULAR_WASM_ASSETS_DIR}/fonts/oldmadina"
    COMMAND "${CMAKE_COMMAND}" -E copy "${INDOPAKFONT_DIR}/libindopak.wasm" "${ANGULAR_WASM_ASSETS_DIR}/fonts/indopak"
    COMMENT "Copying prebuilt WASM font side modules to Angular")
endif()

add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/mfplain.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/mpost.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/vmf.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/madina.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/glyphs.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/features.fea" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/parameters.json" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMENT "Copying to output directory")
