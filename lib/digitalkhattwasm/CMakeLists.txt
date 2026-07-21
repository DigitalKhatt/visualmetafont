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
  set(DIGITALKHATTWASM_STANDALONE TRUE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../harfbuzz ${CMAKE_CURRENT_BINARY_DIR}/harfbuzz)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../mplib ${CMAKE_CURRENT_BINARY_DIR}/mplib)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../digitalkhatt ${CMAKE_CURRENT_BINARY_DIR}/digitalkhatt)
else()
  set(DIGITALKHATTWASM_STANDALONE FALSE)
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

target_link_options(${WEBLIB_NAME} PRIVATE --bind --use-preload-plugins  # -fsanitize=leak
  # madina_font (madinafont/) is dlopen()'d at runtime as a wasm side
  # module (mirroring the native dlopen()-based plugin architecture) --
  # MAIN_MODULE=1 exports this module's full symbol surface, including
  # digitalkhatt_otlayout's statically-linked-in code, so the side module
  # can resolve against it. =1 (not =2) because Madina calls back into the
  # main module throughout its lifetime, not just at construction, making
  # manual EXPORTED_FUNCTIONS enumeration impractical.
  "SHELL:-s MAIN_MODULE=1"
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

  if(NOT DIGITALKHATTWASM_STANDALONE)
    # dynamicLibraries: ['libmadinad.wasm'] (see test/index.html) resolves
    # relative to ${WEBLIB_NAME}.js's own directory by default, so the side
    # module needs to be co-located with it there too, not just in
    # madinafont/. madina_font is a forward reference here -- it's only
    # added later by the root CMakeLists.txt (after add_subdirectory(visualmetafont)
    # returns), which is exactly the case this file wasn't configured
    # standalone for.
    #
    # NOTE: this is a TARGET POST_BUILD command, so it only re-runs when
    # ${WEBLIB_NAME} itself rebuilds -- if you change ONLY madinafont
    # sources, this copy can go stale. An OUTPUT-based add_custom_command
    # (which tracks madina_font's own freshness) was tried here but hits a
    # CMake generator-expression limitation with add_custom_target(... ALL
    # ...) referencing another target's TARGET_FILE_DIR under this
    # multi-config generator ("No target ${WEBLIB_NAME}" at generate time,
    # even in this same directory). If this staleness bites during
    # madinafont-only iteration, manually re-copy
    # madinafont/libmadina(d).wasm next to ${WEBLIB_NAME}.js, or touch a
    # ${WEBLIB_NAME} source file to force its POST_BUILD to re-run.
    add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:madina_font>" "$<TARGET_FILE_DIR:${WEBLIB_NAME}>"
      COMMENT "Copying madina wasm side module next to ${WEBLIB_NAME}")
  endif()

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

# Assets QuranShaper::loadFontFile (quranshaper.h) reads at runtime, mirroring
# Font::loadFile (visualmetafont/src/metafont/font.cpp): the shared MetaPost
# infrastructure files it prepends to any font's source, plus everything that
# makes up the madina font itself (font source, glyph bodies, OT feature
# rules, justification parameters).
set(MADINAFONT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../madinafont")
set(METAFONT_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src/Resources/metafont")

add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/mfplain.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/mpost.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${METAFONT_RESOURCES_DIR}/vmf.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/madina.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/glyphs.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/features.fea" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMAND "${CMAKE_COMMAND}" -E copy "${MADINAFONT_DIR}/parameters.json" "${CMAKE_CURRENT_SOURCE_DIR}/../../../digitalkhatt.org/ClientApp/src/assets"
  COMMENT "Copying to output directory")
