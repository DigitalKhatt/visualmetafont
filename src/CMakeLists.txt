cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project(visualmetafont CXX)

if (NOT EMSCRIPTEN)
  find_package(ZLIB REQUIRED)
#  find_package(PNG REQUIRED)
#  find_package(Freetype REQUIRED)
endif()

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)


cmake_policy(SET CMP0071 NEW)

#add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/showIncludes>)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(WEBLIB_NAME VisualMetaFontWasm)

set(LUATEX_NAME luadigitalkhatt)
set(VMF_SL vmfsl)

if (NOT EMSCRIPTEN)

  find_package(Protobuf REQUIRED)

  find_package(Qt5 COMPONENTS Core OpenGL Svg PrintSupport Widgets Sql REQUIRED)

else()
  set(Qt5_DIR "${QTBuild}/lib/cmake/Qt5")
  set(Qt5Widgets_DIR "${QTBuild}/lib/cmake/Qt5Widgets")
  set(Qt5Gui_DIR "${QTBuild}/lib/cmake/Qt5Gui")
  set(Qt5Core_DIR "${QTBuild}/lib/cmake/Qt5Core")
  set(Qt5OpenGL_DIR "${QTBuild}/lib/cmake/Qt5OpenGL")
  set(Qt5QML_DIR "${QTBuild}/lib/cmake/Qt5QML")
  set(Qt5Svg_DIR "${QTBuild}/lib/cmake/QT5Svg")
  set(Qt5PrintSupport_DIR "${QTBuild}/lib/cmake/Qt5PrintSupport")
  set(Qt5Network_DIR "${QTBuild}/lib/cmake/Qt5Network")
  set(Qt5EventDispatcherSupport_DIR "${QTBuild}/lib/cmake/Qt5EventDispatcherSupport")
  set(Qt5FontDatabaseSupport_DIR "${QTBuild}/lib/cmake/Qt5FontDatabaseSupport")
  set(Qt5Zlib_DIR "${QTBuild}/lib/cmake/Qt5Zlib")
  set(Qt5EglSupport_DIR "${QTBuild}/lib/cmake/Qt5EglSupport")

  find_package(Qt5 COMPONENTS Core OpenGL Svg PrintSupport Widgets EventDispatcherSupport FontDatabaseSupport Zlib REQUIRED)

  get_target_property(
    linked_libs
    Qt5::Widgets
    INTERFACE_LINK_LIBRARIES
    )

  set(gen_expr "${QTBuild}/lib/libqtpcre2.a"
    "${QTBuild}/lib/libqtfreetype.a;"
    "${QTBuild}/lib/libqtlibpng.a;")

  set_target_properties(Qt5::Widgets  PROPERTIES INTERFACE_LINK_LIBRARIES "${gen_expr};${linked_libs}")

endif()

set(Automedina
  automedina/automedina.h
  automedina/automedina.cpp
  automedina/defaultmarkpositions.h
  )

source_group("Automedina" FILES ${Automedina})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GlyphParser)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/FeaParser)


#BISON_TARGET(FeaParser FeaParser/parser.y ${CMAKE_CURRENT_BINARY_DIR}/FeaParser/parser.cpp VERBOSE COMPILE_FLAGS "-Wcounterexamples")
BISON_TARGET(FeaParser FeaParser/parser.y ${CMAKE_CURRENT_BINARY_DIR}/FeaParser/parser.cpp VERBOSE COMPILE_FLAGS)
FLEX_TARGET(FeaScanner FeaParser/scanner.l  ${CMAKE_CURRENT_BINARY_DIR}/FeaParser/scanner.cpp)
ADD_FLEX_BISON_DEPENDENCY(FeaScanner FeaParser)

set(FeaParser
  FeaParser/parser.y
  FeaParser/scanner.l
  FeaParser/driver.cpp
  FeaParser/driver.h
  FeaParser/feaast.cpp
  FeaParser/feaast.h
  FeaParser/scanner.h
  FeaParser/just.h
  FeaParser/just.cpp  
  FeaParser/fsm.cpp
  ${BISON_FeaParser_OUTPUTS}
  ${FLEX_FeaScanner_OUTPUTS}
  )

source_group("FeaParser" FILES ${FeaParser})

BISON_TARGET(GlyphParser GlyphParser/glyphparser.y ${CMAKE_CURRENT_BINARY_DIR}/GlyphParser/glyphparser.cpp VERBOSE)
FLEX_TARGET(GlyphScanner GlyphParser/glyphscanner.l  ${CMAKE_CURRENT_BINARY_DIR}/GlyphParser/glyphscanner.cpp)
ADD_FLEX_BISON_DEPENDENCY(GlyphScanner GlyphParser)

set(GlyphParser
  GlyphParser/glyphdriver.cpp
  GlyphParser/glyphdriver.h
  GlyphParser/glyphscanner.h
  GlyphParser/glyphparser.y
  GlyphParser/glyphscanner.l
  ${BISON_GlyphParser_OUTPUTS}
  ${FLEX_GlyphScanner_OUTPUTS}
  )

source_group("GlyphParser" FILES ${GlyphParser})

set(Export
  Export/ExportToHTML.cpp
  Export/ExportToHTML.h
  Export/GenerateLayout.h
  Export/GenerateLayout.cpp
  Export/protobuf/quran.pb.cc
  Export/protobuf/quran.pb.h
  Export/generateLayoutProtoBuf.cpp
  )
source_group("Export" FILES ${Export})

set(Layout
  Layout/JustificationContext.cpp
  Layout/JustificationContext.h
  #Layout/GlyphItem.cpp
  Layout/GlyphVis.cpp
  Layout/GlyphVis.h
  #Layout/GraphicsSceneAdjustment.cpp
  #Layout/GraphicsViewAdjustment.cpp
  #Layout/LayoutWindow.cpp
  Layout/QByteArrayOperator.cpp
  Layout/QByteArrayOperator.h
  Layout/Lookup.cpp
  Layout/Lookup.h
  Layout/OtLayout.cpp
  Layout/OtLayout.h
  Layout/Subtable.cpp
  Layout/Subtable.h
  Layout/to_opentype.cpp
  Layout/to_opentype.h
  Layout/FSMDriver.h
  Layout/FSMDriver.cpp
  Layout/commontypes.h
  Layout/just_features.cpp
  Layout/gllobal_strings.h
  Layout/gllobal_strings.cpp
  )



set(LayoutWindow  
  Layout/GlyphItem.cpp
  Layout/GraphicsSceneAdjustment.cpp
  Layout/GraphicsViewAdjustment.cpp
  Layout/LayoutWindow.cpp
  Layout/apply_force.cpp
  Layout/LookupEdit.cpp
  )
source_group("Layout" FILES ${LayoutWindow})

if(ENABLE_PDF_GENERATION)
set(Pdf
  Pdf/qpdf.cpp
  Pdf/qpdf.h
  Pdf/quranpdfwriter.cpp
  Pdf/quranpdfwriter.h
  )
endif()

source_group("Pdf" FILES ${Pdf})

set(QuranText
  qurantext/quran.cpp
  qurantext/quran.h
  qurantext/qurancomplex.cpp
  qurantext/surahs.cpp
  )
source_group("QuranText" FILES ${QuranText})

set(Resource_Files
  Resources/mpgui.qrc
  )
source_group("Resource Files" FILES ${Resource_Files})

set(Main
  main.cpp
  )
source_group("" FILES ${Main})

set(FontWindow
  fontwindow/fontscene.cpp
  fontwindow/glyphcellheaderitem.cpp
  fontwindow/glyphcellitem.cpp
  fontwindow/mainwindow.cpp
  fontwindow/mdichild.cpp
  )
source_group("FontWindow" FILES ${FontWindow})

set(GlyphCommands
  glyphwindow/commands.cpp
  glyphwindow/commands.h
)

set(GlyphWindow 
  glyphwindow/componentitem.cpp
  glyphwindow/contouritem.cpp
  glyphwindow/glyphparametercontroller.cpp
  glyphwindow/glyphscene.cpp
  glyphwindow/glyphview.cpp
  glyphwindow/glyphwindow.cpp
  glyphwindow/guidesitem.cpp
  glyphwindow/imageitem.cpp
  glyphwindow/knotcontrolleditem.cpp
  glyphwindow/knotitem.cpp
  glyphwindow/pairitem.cpp
  glyphwindow/ruleritem.cpp
  glyphwindow/pathview.h
  glyphwindow/pathview.cpp
  )
source_group("GlyphWindow" FILES ${GlyphWindow})

set(MetaFont
  metafont/font.cpp
  metafont/font.hpp
  metafont/glyph.cpp
  metafont/glyph.hpp
  metafont/exp.hpp
  metafont/metafont.h
  )
source_group("MetaFont" FILES ${MetaFont})



set(ALL_FILES ${Automedina} ${GlyphCommands} ${FeaParser} ${MetaFont} ${GlyphParser} ${QuranText} ${Layout} 
  ${GlyphWindow}  ${FontWindow}
  ${Export} ${LayoutWindow} ${Main} ${Pdf}  ${Generated_Files} ${Resource_Files}
)


set(WEBLIB_FILES  ${Automedina} ${FeaParser}  
  ${Layout} ${QuranText} ${Header_Files} ${Resource_Files} weblib/visualmetafontwasm.cpp weblib/quranshaper.h)

set(LUATEX_FILES
#${Automedina} ${FeaParser} ${Layout} ${QuranText} ${Header_Files} ${Resource_Files}
  luatex/luadigitalkhatt/blob.c luatex/luadigitalkhatt/buffer.c luatex/luadigitalkhatt/class_utils.c luatex/luadigitalkhatt/class_utils.c
  luatex/luadigitalkhatt/direction.c luatex/luadigitalkhatt/face.c luatex/luadigitalkhatt/feature.c
  luatex/luadigitalkhatt/font.c luatex/luadigitalkhatt/language.c luatex/luadigitalkhatt/luadigitalkhatt.c
  luatex/luadigitalkhatt/luadigitalkhatt.h luatex/luadigitalkhatt/ot.c luatex/luadigitalkhatt/script.c luatex/luadigitalkhatt/tag.c
  luatex/luadigitalkhatt/unicode.c luatex/luadigitalkhatt/variation.c)

add_executable(${WEBLIB_NAME} ${WEBLIB_FILES})

add_library(${LUATEX_NAME} SHARED  ${LUATEX_FILES})

add_executable(${PROJECT_NAME} ${ALL_FILES})


add_library(${VMF_SL} SHARED  ${Automedina} ${GlyphCommands} ${FeaParser} ${MetaFont} ${GlyphParser} ${QuranText} ${Layout})

target_include_directories(${VMF_SL} PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/."
  "${CMAKE_CURRENT_SOURCE_DIR}/Layout"
  "${CMAKE_CURRENT_SOURCE_DIR}/FeaParser"
  "${CMAKE_CURRENT_SOURCE_DIR}/GlyphParser"
  "${CMAKE_CURRENT_SOURCE_DIR}/metafont"
  "${CMAKE_CURRENT_SOURCE_DIR}/fontwindow"
  "${CMAKE_CURRENT_SOURCE_DIR}/glyphwindow"
  "${CMAKE_CURRENT_SOURCE_DIR}/Automedina"
)
target_include_directories(${VMF_SL} PRIVATE ${FLEX_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/FeaParser ${CMAKE_CURRENT_BINARY_DIR}/GlyphParser)
add_dependencies(${VMF_SL} harfbuzz mplib)
target_compile_options(${VMF_SL} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
target_compile_definitions(${VMF_SL} PRIVATE HB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR)
target_link_libraries(${VMF_SL} PUBLIC harfbuzz mplib Qt5::Core Qt5::Gui Qt5::Widgets)
#add_dependencies(${PROJECT_NAME} ${VMF_SL})
set_target_properties(${VMF_SL}  PROPERTIES ENABLE_EXPORTS 1 WINDOWS_EXPORT_ALL_SYMBOLS 1)
set_target_properties(${PROJECT_NAME}  PROPERTIES ENABLE_EXPORTS 1 WINDOWS_EXPORT_ALL_SYMBOLS 1)
export(TARGETS ${PROJECT_NAME} harfbuzz mplib QtPropertyBrowser FILE ${CMAKE_CURRENT_BINARY_DIR}/visualmetafontConfig.cmake)
#export(TARGETS ${VMF_SL} harfbuzz mplib QtPropertyBrowser FILE ${CMAKE_CURRENT_BINARY_DIR}/vmfsl.cmake)




target_include_directories(${PROJECT_NAME} PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/."
  "${CMAKE_CURRENT_SOURCE_DIR}/Layout"
  "${CMAKE_CURRENT_SOURCE_DIR}/FeaParser"
  "${CMAKE_CURRENT_SOURCE_DIR}/GlyphParser"
  "${CMAKE_CURRENT_SOURCE_DIR}/metafont"
  "${CMAKE_CURRENT_SOURCE_DIR}/fontwindow"
  "${CMAKE_CURRENT_SOURCE_DIR}/glyphwindow"
  "${CMAKE_CURRENT_SOURCE_DIR}/Automedina"
  )

target_include_directories(${WEBLIB_NAME} PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/."
  "${CMAKE_CURRENT_SOURCE_DIR}/Layout"
  "${CMAKE_CURRENT_SOURCE_DIR}/FeaParser"
  "${CMAKE_CURRENT_SOURCE_DIR}/GlyphParser"
  "${CMAKE_CURRENT_SOURCE_DIR}/metafont"
  "${CMAKE_CURRENT_SOURCE_DIR}/fontwindow"
  "${CMAKE_CURRENT_SOURCE_DIR}/glyphwindow"
  "${CMAKE_CURRENT_SOURCE_DIR}/Automedina"
  )

add_library(lualib SHARED IMPORTED)

if (WIN32)
    set_property(TARGET lualib PROPERTY IMPORTED_IMPLIB  ${IMPLIBLOCATION})
endif (WIN32)

if (UNIX)
    set_property(TARGET lualib PROPERTY IMPORTED_LOCATION  ${IMPLIBLOCATION})   
endif (UNIX)



target_include_directories(${LUATEX_NAME} PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/."
  "${CMAKE_CURRENT_SOURCE_DIR}/Layout"
  "${CMAKE_CURRENT_SOURCE_DIR}/FeaParser"
  "${CMAKE_CURRENT_SOURCE_DIR}/GlyphParser"
  "${CMAKE_CURRENT_SOURCE_DIR}/metafont"
  "${CMAKE_CURRENT_SOURCE_DIR}/fontwindow"
  "${CMAKE_CURRENT_SOURCE_DIR}/glyphwindow"
  "${CMAKE_CURRENT_SOURCE_DIR}/luatex/luaharfbuzz"
  ${LUA_INCLUDE_DIR}
  )

target_include_directories(${PROJECT_NAME} PRIVATE ${FLEX_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/FeaParser ${CMAKE_CURRENT_BINARY_DIR}/GlyphParser)
target_include_directories(${WEBLIB_NAME} PRIVATE ${FLEX_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/FeaParser ${CMAKE_CURRENT_BINARY_DIR}/GlyphParser)
target_include_directories(${LUATEX_NAME} PRIVATE ${FLEX_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/FeaParser ${CMAKE_CURRENT_BINARY_DIR}/GlyphParser)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
target_compile_options(${WEBLIB_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
target_compile_options(${LUATEX_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)


target_compile_definitions(${PROJECT_NAME} PRIVATE HB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR COMPILING_DLL)
target_compile_definitions(${WEBLIB_NAME} PRIVATE HB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR DIGITALKHATT_WEBLIB COMPILING_DLL)
target_compile_definitions(${LUATEX_NAME} PRIVATE HB_NO_PRAGMA_GCC_DIAGNOSTIC_ERROR COMPILING_DLL)


if(ENABLE_PDF_GENERATION)
target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_PDF_GENERATION)
endif()

if (QT_SHARED_BUILD)
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_SHARED_BUILD)
else()
set(QT_STATIC_PLUGINS
  Qt5::QJpegPlugin
  Qt5::QSvgPlugin
  Qt5::QSvgIconPlugin
)
endif()

add_dependencies(${PROJECT_NAME} harfbuzz mplib QtPropertyBrowser)
add_dependencies(${LUATEX_NAME} harfbuzz mplib)

if (UNIX)
  set_target_properties(${LUATEX_NAME}  PROPERTIES PREFIX "" IMPORT_PREFIX "")
endif (UNIX)


set_target_properties(${LUATEX_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden)
target_link_libraries(${LUATEX_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wl,--exclude-libs,ALL>)
#target_link_libraries(${LUATEX_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-static-libgcc>)
#target_link_libraries(${LUATEX_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>)

if (NOT EMSCRIPTEN)
  # Link with other targets.
  target_link_libraries(${PROJECT_NAME} PUBLIC harfbuzz mplib PRIVATE  QtPropertyBrowser     
    Qt5::Core Qt5::CorePrivate Qt5::GuiPrivate Qt5::Gui Qt5::Svg Qt5::PrintSupport Qt5::Widgets Qt5::Sql
    ${QT_STATIC_PLUGINS}
    ZLIB::ZLIB
  )
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC harfbuzz mplib QtPropertyBrowser     
    Qt5::Core Qt5::CorePrivate Qt5::GuiPrivate Qt5::Gui Qt5::Svg Qt5::PrintSupport Qt5::Widgets
    Qt5::QJpegPlugin
    Qt5::QSvgPlugin
    Qt5::QSvgIconPlugin
    )
endif()

target_link_libraries(${WEBLIB_NAME} PUBLIC harfbuzz mplib Qt5::Core)

target_link_libraries(${LUATEX_NAME} PUBLIC harfbuzz mplib Qt5::Core)

target_link_libraries(${LUATEX_NAME} PRIVATE lualib)

if (NOT EMSCRIPTEN)

  if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE imm32 version Dwmapi Mincore winmm Wtsapi32)
    if (NOT QT_SHARED_BUILD)
      target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::QWindowsIntegrationPlugin)
    endif()
    target_link_libraries(${WEBLIB_NAME} PUBLIC imm32 version Dwmapi Mincore winmm Wtsapi32 Qt5::QWindowsIntegrationPlugin)
  endif (WIN32) 

  target_link_libraries(${PROJECT_NAME} PRIVATE protobuf::libprotobuf)

else()

  target_compile_options(${WEBLIB_NAME} PUBLIC -w)

  target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::QWasmIntegrationPlugin Qt5::EventDispatcherSupport Qt5::FontDatabaseSupport)

  target_compile_options(${PROJECT_NAME} PRIVATE
    "SHELL:-s USE_ZLIB=1"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    # "SHELL:-s USE_FREETYPE=1"
    # "SHELL:-s USE_LIBPNG=1"
    )

  target_compile_options(${WEBLIB_NAME} PRIVATE
    # -fsanitize=leak
    "SHELL:-s USE_ZLIB=1"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    # "SHELL:-s USE_FREETYPE=1"
    # "SHELL:-s USE_LIBPNG=1"
    )

  target_link_options(${PROJECT_NAME} PRIVATE --bind
    "SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=0"
    #"SHELL:-s WASM=1"
    "SHELL:-s FULL_ES2=1"
    "SHELL:-s USE_WEBGL2=1"
    "SHELL:-s NO_EXIT_RUNTIME=0"
    # "SHELL:-s \"BINARYEN_TRAP_MODE='clamp'\""
    "SHELL:-s ALLOW_MEMORY_GROWTH=1" -O3
    "SHELL:-s USE_ZLIB=1"
    "SHELL:-s EXPORT_ES6=1"
    "SHELL:-s USE_ES6_IMPORT_META=0"
    "SHELL:-s \"EXTRA_EXPORTED_RUNTIME_METHODS=['FS','callMain']\""
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s \"EXPORTED_FUNCTIONS=['_main']\""
    #--post-js "${CMAKE_CURRENT_SOURCE_DIR}/post-js.js"
    )

  target_link_options(${WEBLIB_NAME} PRIVATE --bind --use-preload-plugins  # -fsanitize=leak
    "SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=1"
    #"SHELL:-s WASM=1"
    #"SHELL:-s WASM=0 --separate-asm -s \"SEPARATE_ASM_MODULE_NAME='VMetaFont'\""
    #"SHELL:-s ASSERTIONS=1"
    #"SHELL:-s FULL_ES2=1"
    #"SHELL:-s USE_WEBGL2=1"
    "SHELL:-s USE_ES6_IMPORT_META=0"
    "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
    "SHELL:-s NO_EXIT_RUNTIME=0"
    # "SHELL:-s \"BINARYEN_TRAP_MODE='clamp'\""
    "SHELL:-s ALLOW_MEMORY_GROWTH=1" #-O3
    "SHELL:-s USE_ZLIB=1"
    #"SHELL:-s USE_SDL=2"
    #"SHELL:-s EXPORT_ES6=1"
    "SHELL:-s \"EXPORTED_RUNTIME_METHODS=['FS','print','printErr']\""
    #"SHELL:-s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[\\\"$Browser\\\"]"
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s IMPORTED_MEMORY"    
    #"SHELL:-s TOTAL_MEMORY=1048576000"
    "SHELL:-s ENVIRONMENT=web"    
    #"SHELL:-s \"EXPORTED_FUNCTIONS=['_main']\""
    #--post-js "${CMAKE_CURRENT_SOURCE_DIR}/post-js.js"
    )

  if(WASMTYPE STREQUAL "WASM")

    target_link_options(${PROJECT_NAME} PRIVATE
      "SHELL:-s WASM=1"
      "SHELL:-s EXPORT_ES6=1"      
      )

    target_link_options(${WEBLIB_NAME} PRIVATE
      "SHELL:-s WASM=1"
      "SHELL:-s EXPORT_ES6=1"
      "SHELL:-s DEMANGLE_SUPPORT=1"      
      )

    add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/${WEBLIB_NAME}.js" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/app/services/quranservice"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/${WEBLIB_NAME}.wasm" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"
      COMMENT "Copying to output directory")

    #add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #	COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.js" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/app/visualmetafont"
    #	COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.wasm" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"
    #	COMMENT "Copying to output directory")

  else()
    target_link_options(${WEBLIB_NAME} PRIVATE
      "SHELL:-s WASM=0"
      "SHELL:-s EXPORT_NAME='VMASM'"
      "SHELL:-s TOTAL_MEMORY=26214400"
      )

    add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/${WEBLIB_NAME}.js"		"${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/vmasm"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/${WEBLIB_NAME}.js.mem"	"${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/vmasm"
      COMMENT "Copying to output directory")

    #add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #	COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/${WEBLIB_NAME}.js"		"${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/vmasm"
    #	COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/${WEBLIB_NAME}.js.mem"	"${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/vmasm"
    #	COMMENT "Copying to output directory")


  endif()

  
  add_custom_command(TARGET ${WEBLIB_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../files/features.fea" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../files/ayah.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../files/mfplain.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../files/digitalkhatt.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../files/mpguifont.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../files/myfontbase.mp" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"    
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../files/parameters.json" "${CMAKE_CURRENT_SOURCE_DIR}/../../digitalkhatt.org/ClientApp/src/assets"
    COMMENT "Copying to output directory")
  

endif()


include(CPack)
