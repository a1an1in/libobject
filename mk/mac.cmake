macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
        set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/mac)
    endif ()
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
    LINK_DIRECTORIES(/usr/local/lib 
        /usr/lib
        ${CMAKE_INSTALL_PREFIX}/lib
        /usr/local/opt/mysql-client/lib
        /usr/local/Cellar/openssl@1.1/1.1.1i/lib
        )

    INCLUDE_DIRECTORIES(/usr/local/include
        /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include
        /usr/local/opt/mysql-client/include/mysql
        /usr/local/Cellar/openssl@1.1/1.1.1i/include
        ${PROJECT_SOURCE_DIR}/src/include
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/sysroot/mac/include)

    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
    set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    set (EXTERNAL_LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/mac)
    set (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
    SET(ExternalLibs ${ExternalLibs} 
        -force_load object-tests
        #-force_load object-archive
        -force_load object-argument
        -force_load object-cmds
        #-force_load object-compress
        -force_load object-concurrent
        -force_load object-core
        -force_load object-crypto
        -force_load object-ctest
        -force_load object-db
        -force_load object-encoding
        #-force_load object-media
        -force_load object-message
        -force_load object-net
        -force_load object-scripts
        -force_load object-stub
        #-force_load object-ui
        crypto z iconv bz2 )

    #SET(ExternalLibs ${ExternalLibs} -force_load object-ex-media -force_load object-ex-ui avformat avfilter swscale swresample avcodec avutil x264 vorbis vorbisenc vorbisfile mp3lame vpx xvidcore opus fdk-aac theora xvidcore SDL2 SDL2_ttf yuv)
    find_library(SECURITY Security)
    if (NOT SECURITY)
        message(FATAL_ERROR "Security not found")
    endif()

    find_library(CORE_MEDIA CoreMedia)
    if (NOT CORE_MEDIA)
        message(FATAL_ERROR "CoreMedia not found")
    endif()

    find_library(VIDEO_TOOL_BOX VideoToolBox)
    if (NOT VIDEO_TOOL_BOX)
        message(FATAL_ERROR "VideoToolBox not found")
    endif()

    find_library(Foundation_Path Foundation)
    if (NOT Foundation_Path)
        message(FATAL_ERROR "Foundation_Path not found")
    else ()
        message("Foundation_Path: ${Foundation_Path}")
    endif()

    find_library(AudioToolbox_Path AudioToolbox)
    if (NOT AudioToolbox_Path)
        message(FATAL_ERROR "AudioToolbox_Path not found")
    else ()
        message("AudioToolbox_Path: ${AudioToolbox_Path}")
    endif()

    find_library(CoreGraphics_Path CoreGraphics)
    if (NOT CoreGraphics_Path)
        message(FATAL_ERROR "CoreGraphics_Path not found")
    else ()
        message("CoreGraphics_Path: ${CoreGraphics_Path}")
    endif()

    find_library(QuartzCore_Path QuartzCore)
    if (NOT QuartzCore_Path)
        message(FATAL_ERROR "QuartzCore_Path not found")
    else ()
        message("QuartzCore_Path: ${QuartzCore_Path}")
    endif()

    find_library(OpenAL_Path OpenAL)
    if (NOT OpenAL_Path)
        message(FATAL_ERROR "OpenAL_Path not found")
    else ()
        message("OpenAL_Path: ${OpenAL_Path}")
    endif()

    find_library(mediaToolbox_Path mediaToolbox)
    if (NOT mediaToolbox_Path)
        message(FATAL_ERROR "mediaToolbox_Path not found")
    else ()
        message("mediaToolbox_Path: ${mediaToolbox_Path}")
    endif()

    SET(ExternalLibs ${ExternalLibs} ${CORE_MEDIA} ${VIDEO_TOOL_BOX} ${SECURITY}
        ${Foundation_Path} ${AudioToolbox_Path} ${CoreGraphics_Path} ${QuartzCore_Path} ${OpenAL_Path} ${mediaToolbox_Path} )

endmacro()

macro (display_mac_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
