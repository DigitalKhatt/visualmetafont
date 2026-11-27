cmake_minimum_required(VERSION 3.20)
project(digitalkhatt_engine VERSION 0.1.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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

add_library(digitalkhatt_engine src/engine.cpp)
add_library(digitalkhatt::engine ALIAS digitalkhatt_engine)
target_compile_features(digitalkhatt_engine PRIVATE cxx_std_17)
target_compile_definitions(digitalkhatt_engine PRIVATE DIGITALKHATT_ENGINE_BUILDING)
target_include_directories(digitalkhatt_engine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(digitalkhatt_engine PRIVATE harfbuzz)
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
  target_compile_features(digitalkhatt_native_conformance PRIVATE cxx_std_17)
  target_link_libraries(digitalkhatt_native_conformance
    PRIVATE digitalkhatt::engine harfbuzz Threads::Threads
  )
  add_test(
    NAME digitalkhatt_native_conformance
    COMMAND digitalkhatt_native_conformance
      "${DIGITALKHATT_ENGINE_TEST_FONT}"
  )
endif()
