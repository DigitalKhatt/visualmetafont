# Native font plugins use the same link interface in the workspace and after installation.
set(otlayout_targets digitalkhatt_otlayout digitalkhatt_core digitalkhatt_justify
  digitalkhatt_geometry digitalkhatt_layout harfbuzz mplib)

install(TARGETS ${otlayout_targets} EXPORT digitalkhatt_otlayoutTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/digitalkhatt")
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "${OTLAYOUT_ROOT}/Layout" "${OTLAYOUT_ROOT}/automedina"
  "${OTLAYOUT_ROOT}/FeaParser" "${OTLAYOUT_ROOT}/qurantext"
  DESTINATION "${otlayout_install_include}" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "${OTLAYOUT_GENERATED_DIR}/" DESTINATION "${otlayout_install_include}/FeaParser"
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.hh")
install(FILES "${FLEX_INCLUDE_DIR}/FlexLexer.h" DESTINATION "${otlayout_install_include}/FeaParser")

get_target_property(mplib_source_dir mplib SOURCE_DIR)
install(DIRECTORY "${mplib_source_dir}/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/digitalkhatt/mplib"
  FILES_MATCHING PATTERN "*.h")
get_target_property(harfbuzz_source_dir harfbuzz SOURCE_DIR)
file(GLOB harfbuzz_headers CONFIGURE_DEPENDS "${harfbuzz_source_dir}/harfbuzz/src/hb*.h")
install(FILES ${harfbuzz_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/digitalkhatt/harfbuzz")

# A build-tree package can use fetched libraries directly. System packages
# must be discovered again in the consumer. Keep the existing unqualified
# build-tree targets available to callers that already use them.
set(build_targets ${otlayout_targets} pcre2-16-static)
set(package_test_dependency_dirs)
get_target_property(find_clipper2 Clipper2::Clipper2 IMPORTED)
if(NOT find_clipper2)
  list(APPEND build_targets Clipper2)
elseif(Clipper2_DIR)
  list(APPEND package_test_dependency_dirs "-DClipper2_DIR=${Clipper2_DIR}")
endif()
get_target_property(find_glaze glaze::glaze IMPORTED)
if(NOT find_glaze)
  list(APPEND build_targets glaze_glaze)
elseif(glaze_DIR)
  list(APPEND package_test_dependency_dirs "-Dglaze_DIR=${glaze_DIR}")
endif()
set(find_pcre2 OFF)
set(imported_otlayout digitalkhatt_otlayout)
set(package_config digitalkhatt_otlayoutConfig.cmake)
export(TARGETS ${build_targets} FILE "${CMAKE_CURRENT_BINARY_DIR}/digitalkhatt_otlayoutTargets.cmake")
configure_file("cmake/${package_config}.in" "${CMAKE_CURRENT_BINARY_DIR}/${package_config}" @ONLY)
export(PACKAGE digitalkhatt_otlayout)

# Installed dependencies provide their own relocatable CMake targets.
set(find_clipper2 ON)
set(find_glaze ON)
set(find_pcre2 ON)
set(imported_otlayout digitalkhatt::digitalkhatt_otlayout)
set(package_dir "${CMAKE_INSTALL_LIBDIR}/cmake/digitalkhatt_otlayout")
configure_file("cmake/${package_config}.in" "${CMAKE_CURRENT_BINARY_DIR}/install/${package_config}" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/install/${package_config}" DESTINATION "${package_dir}")
install(EXPORT digitalkhatt_otlayoutTargets NAMESPACE digitalkhatt:: DESTINATION "${package_dir}")

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
  set(DIGITALKHATT_PACKAGE_TEST_FONT "" CACHE PATH "Font project to build against the installed native engine")
  set(package_test_font "${DIGITALKHATT_PACKAGE_TEST_FONT}")
  if(NOT package_test_font AND EXISTS "${CMAKE_SOURCE_DIR}/madinafont/CMakeLists.txt")
    set(package_test_font "${CMAKE_SOURCE_DIR}/madinafont")
  endif()
  if(package_test_font)
    configure_file(tests/package/installed-font.cmake.in installed-font.cmake @ONLY)
    add_test(NAME digitalkhatt.installed_font_plugin COMMAND "${CMAKE_COMMAND}"
      "-Dconfig=$<CONFIG>" -P "${CMAKE_CURRENT_BINARY_DIR}/installed-font.cmake")
  endif()
endif()
