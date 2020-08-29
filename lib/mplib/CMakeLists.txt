cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(mplib C)

set(AddedFiles
    AddedFiles/newmp.c
    AddedFiles/newmp.h
)

source_group("AddedFiles" FILES ${AddedFiles})

set(Header_Files
    avl.h
    decContext.h
    decNumber.h
    decNumberLocal.h
    mplib.h
    mplibpng.h
    mplibps.h
    mplibsvg.h
    mpmath.h
    mpmathbinary.h
    mpmathdecimal.h
    mpmathdouble.h
    mpmp.h
    mppngout.h
    mppsout.h
    mpstrings.h
    mpsvgout.h
    mpxout.h    
    w2c/c-auto.h
    w2c/config.h
)

source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    avl.c
    decContext.c
    decNumber.c
    mpmath.c
    #mpmathbinary.c
    mpmathdecimal.c
    mpmathdouble.c
    mpstrings.c
    #mpxout.c
    #pngout.c
    psout.c
    svgout.c
    tfmin.c
)

source_group("Source Files" FILES ${Source_Files})

add_library(${PROJECT_NAME} STATIC ${AddedFiles} ${Header_Files}  ${Source_Files})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
