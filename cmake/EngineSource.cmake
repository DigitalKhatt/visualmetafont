# An empty value can also come from a build configured before the extraction.
if(NOT DIGITALKHATT_ENGINE_SOURCE_DIR)
  get_filename_component(DIGITALKHATT_ENGINE_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../libdigitalkhatt" ABSOLUTE)
endif()
set(DIGITALKHATT_ENGINE_SOURCE_DIR "${DIGITALKHATT_ENGINE_SOURCE_DIR}" CACHE PATH
  "Path to the libdigitalkhatt checkout" FORCE)

if(NOT EXISTS "${DIGITALKHATT_ENGINE_SOURCE_DIR}/engine/CMakeLists.txt")
  message(FATAL_ERROR "Cannot find the DigitalKhatt engine in '${DIGITALKHATT_ENGINE_SOURCE_DIR}'. "
    "Place the libdigitalkhatt checkout beside visualmetafont or configure with "
    "-DDIGITALKHATT_ENGINE_SOURCE_DIR=/path/to/libdigitalkhatt.")
endif()
if(NOT EXISTS "${DIGITALKHATT_ENGINE_SOURCE_DIR}/lib/harfbuzz/harfbuzz/src/hb.h")
  message(FATAL_ERROR "Initialize the engine's HarfBuzz submodule with "
    "git -C \"${DIGITALKHATT_ENGINE_SOURCE_DIR}\" submodule update --init --recursive.")
endif()
