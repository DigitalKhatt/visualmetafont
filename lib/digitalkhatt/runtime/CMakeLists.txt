cmake_minimum_required(VERSION 3.20)
project(digitalkhatt_engine VERSION 0.2.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(_digitalkhatt_engine_tests_default OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(_digitalkhatt_engine_tests_default ON)
endif()
option(DIGITALKHATT_ENGINE_BUILD_TESTS
       "Build the native DigitalKhatt runtime conformance tests"
       ${_digitalkhatt_engine_tests_default})
unset(_digitalkhatt_engine_tests_default)

if(NOT TARGET harfbuzz)
  set(HB_BUILD_SUBSET OFF CACHE BOOL "" FORCE)
  set(HB_BUILD_UTILS OFF CACHE BOOL "" FORCE)
  add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../harfbuzz"
    "${CMAKE_CURRENT_BINARY_DIR}/harfbuzz"
  )
endif()

if(TARGET digitalkhatt_justify)
  set(_digitalkhatt_engine_justify_target digitalkhatt_justify)
else()
  include(FetchContent)
  function(_digitalkhatt_engine_add_pcre2)
    FetchContent_Declare(
      pcre2
      GIT_REPOSITORY https://github.com/PCRE2Project/pcre2.git
      GIT_TAG pcre2-10.47
      # Populate through FetchContent, then add it ourselves with
      # EXCLUDE_FROM_ALL so its install rules do not leak into this SDK.
      SOURCE_SUBDIR digitalkhatt-no-auto-add
    )
    set(PCRE2_BUILD_PCRE2_8 OFF CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_PCRE2_16 ON CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_PCRE2_32 OFF CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(PCRE2_BUILD_PCRE2GREP OFF CACHE BOOL "" FORCE)
    set(PCRE2_SUPPORT_LIBEDIT OFF CACHE BOOL "" FORCE)
    set(PCRE2_SUPPORT_LIBREADLINE OFF CACHE BOOL "" FORCE)
    set(PCRE2_STATIC_PIC ON CACHE BOOL "" FORCE)
    # Function scope prevents dependency settings from changing the parent.
    set(BUILD_STATIC_LIBS ON)
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_MakeAvailable(pcre2)
    add_subdirectory(
      "${pcre2_SOURCE_DIR}" "${pcre2_BINARY_DIR}" EXCLUDE_FROM_ALL
    )
  endfunction()
  _digitalkhatt_engine_add_pcre2()

  add_library(digitalkhatt_engine_core STATIC
    ../src/Regex16.cpp
  )
  target_compile_features(digitalkhatt_engine_core PUBLIC cxx_std_20)
  target_include_directories(digitalkhatt_engine_core PUBLIC ../include)
  target_link_libraries(digitalkhatt_engine_core PRIVATE pcre2-16-static)

  add_library(digitalkhatt_engine_justify STATIC
    ../src/justify/FeatureJustifier.cpp
  )
  target_compile_features(digitalkhatt_engine_justify PUBLIC cxx_std_20)
  target_compile_definitions(digitalkhatt_engine_justify
    PRIVATE DIGITALKHATT_JUSTIFIER_NO_TAJWEED
  )
  target_include_directories(digitalkhatt_engine_justify PUBLIC ../include)
  target_link_libraries(digitalkhatt_engine_justify
    PUBLIC harfbuzz
    PRIVATE digitalkhatt_engine_core
  )
  set(_digitalkhatt_engine_justify_target digitalkhatt_engine_justify)
endif()

add_library(digitalkhatt_engine src/engine.cpp)
add_library(digitalkhatt::engine ALIAS digitalkhatt_engine)
target_compile_features(digitalkhatt_engine PRIVATE cxx_std_20)
target_compile_definitions(digitalkhatt_engine PRIVATE DIGITALKHATT_ENGINE_BUILDING)
target_include_directories(digitalkhatt_engine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(digitalkhatt_engine
  PRIVATE harfbuzz ${_digitalkhatt_engine_justify_target}
)
set_target_properties(digitalkhatt_engine PROPERTIES
  CXX_EXTENSIONS OFF
  CXX_VISIBILITY_PRESET hidden
  EXPORT_NAME engine
  POSITION_INDEPENDENT_CODE ON
  SOVERSION 0
  VERSION ${PROJECT_VERSION}
  VISIBILITY_INLINES_HIDDEN ON
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(digitalkhatt_engine
    PUBLIC DIGITALKHATT_ENGINE_SHARED
  )
  if(APPLE)
    target_link_options(digitalkhatt_engine PRIVATE
      "LINKER:-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/exports.apple"
    )
    set_property(TARGET digitalkhatt_engine APPEND PROPERTY
      LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/exports.apple"
    )
  elseif(UNIX)
    target_link_options(digitalkhatt_engine PRIVATE
      "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports.map"
    )
    set_property(TARGET digitalkhatt_engine APPEND PROPERTY
      LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/exports.map"
    )
  endif()
endif()

install(TARGETS digitalkhatt_engine
  EXPORT DigitalKhattEngineTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES include/digitalkhatt/engine.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/digitalkhatt
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../../LICENSE
  DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/digitalkhatt-engine
)
if(BUILD_SHARED_LIBS)
  set(DIGITALKHATT_ENGINE_CMAKE_DIR
      ${CMAKE_INSTALL_LIBDIR}/cmake/DigitalKhattEngine)
  configure_package_config_file(
    cmake/DigitalKhattEngineConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DigitalKhattEngineConfig.cmake
    INSTALL_DESTINATION ${DIGITALKHATT_ENGINE_CMAKE_DIR}
  )
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/DigitalKhattEngineConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  install(EXPORT DigitalKhattEngineTargets
    FILE DigitalKhattEngineTargets.cmake
    NAMESPACE digitalkhatt::
    DESTINATION ${DIGITALKHATT_ENGINE_CMAKE_DIR}
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DigitalKhattEngineConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DigitalKhattEngineConfigVersion.cmake
    DESTINATION ${DIGITALKHATT_ENGINE_CMAKE_DIR}
  )
endif()

if(DIGITALKHATT_ENGINE_BUILD_TESTS)
  enable_testing()
  set(DIGITALKHATT_ENGINE_TEST_FONT
      "${CMAKE_CURRENT_SOURCE_DIR}/../../../examples/testvarfont/digitalkhatt-cff2.otf"
      CACHE FILEPATH "DigitalKhatt font used by the runtime conformance test")
  set(DIGITALKHATT_ENGINE_PAGE_PARITY_FONT "" CACHE FILEPATH
      "Corrected runtime font used by the strict page-43 parity test")
  if(NOT EXISTS "${DIGITALKHATT_ENGINE_TEST_FONT}")
    message(FATAL_ERROR
      "DIGITALKHATT_ENGINE_TEST_FONT does not exist: "
      "${DIGITALKHATT_ENGINE_TEST_FONT}")
  endif()

  add_executable(digitalkhatt_c_abi_compile tests/c_abi_compile.c)
  target_compile_features(digitalkhatt_c_abi_compile PRIVATE c_std_17)
  set_target_properties(digitalkhatt_c_abi_compile PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(digitalkhatt_c_abi_compile PRIVATE digitalkhatt::engine)
  add_test(NAME digitalkhatt_c_abi_compile COMMAND digitalkhatt_c_abi_compile)

  find_package(Threads REQUIRED)
  add_executable(digitalkhatt_native_conformance tests/native_conformance.cpp)
  target_compile_features(digitalkhatt_native_conformance PRIVATE cxx_std_20)
  target_link_libraries(digitalkhatt_native_conformance
    PRIVATE digitalkhatt::engine harfbuzz Threads::Threads
  )
  add_test(
    NAME digitalkhatt_native_conformance
    COMMAND digitalkhatt_native_conformance
      "${DIGITALKHATT_ENGINE_TEST_FONT}"
  )

  add_executable(digitalkhatt_page_conformance tests/page_conformance.cpp)
  target_compile_features(digitalkhatt_page_conformance PRIVATE cxx_std_20)
  target_link_libraries(digitalkhatt_page_conformance
    PRIVATE digitalkhatt::engine Threads::Threads
  )
  add_test(
    NAME digitalkhatt_page_conformance
    COMMAND digitalkhatt_page_conformance
      "${DIGITALKHATT_ENGINE_TEST_FONT}"
  )
  if(DIGITALKHATT_ENGINE_PAGE_PARITY_FONT)
    if(NOT EXISTS "${DIGITALKHATT_ENGINE_PAGE_PARITY_FONT}")
      message(FATAL_ERROR
        "DIGITALKHATT_ENGINE_PAGE_PARITY_FONT does not exist: "
        "${DIGITALKHATT_ENGINE_PAGE_PARITY_FONT}")
    endif()
    add_test(
      NAME digitalkhatt_page43_parity
      COMMAND digitalkhatt_page_conformance
        "${DIGITALKHATT_ENGINE_PAGE_PARITY_FONT}" --strict
    )
  endif()
endif()
