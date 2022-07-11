cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(QtPropertyBrowser CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Source_Files
    qtbuttonpropertybrowser.cpp
    qteditorfactory.cpp
    qtgroupboxpropertybrowser.cpp
    qtpropertybrowser.cpp
    qtpropertybrowserutils.cpp
    qtpropertymanager.cpp
    qttreepropertybrowser.cpp
    qtvariantproperty.cpp
)

if (NOT EMSCRIPTEN)
	find_package(Qt5 COMPONENTS Widgets REQUIRED)
else()
	set(Qt5_DIR "${QTBuild}/lib/cmake/Qt5")
	set(Qt5Widgets_DIR "${QTBuild}/lib/cmake/Qt5Widgets")
	set(Qt5Gui_DIR "${QTBuild}/lib/cmake/Qt5Gui")
	set(Qt5Core_DIR "${QTBuild}/lib/cmake/Qt5Core")
  set(Qt5Svg_DIR "${QTBuild}/lib/cmake/QT5Svg")
  set(Qt5Zlib_DIR "${QTBuild}/lib/cmake/Qt5Zlib")
  set(Qt5Network_DIR "${QTBuild}/lib/cmake/Qt5Network")
  set(Qt5EventDispatcherSupport_DIR "${QTBuild}/lib/cmake/Qt5EventDispatcherSupport")
  set(Qt5FontDatabaseSupport_DIR "${QTBuild}/lib/cmake/Qt5FontDatabaseSupport")
  set(Qt5EglSupport_DIR "${QTBuild}/lib/cmake/Qt5EglSupport")
	find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

add_library(${PROJECT_NAME} STATIC ${Source_Files})
target_compile_definitions(${PROJECT_NAME} PRIVATE "UNICODE;QTPROPERTYBROWSER_LIB")

target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/.")
