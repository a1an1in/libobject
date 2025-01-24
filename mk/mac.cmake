macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
        SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/mac)
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

    SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
    SET (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    SET (EXTERNAL_LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/mac)
    SET (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

    SET(ExternalLibs ${ExternalLibs} 
        -force_load object-tests
        #-force_load object-archive
        -force_load object-argument
        -force_load object-mockery
        -force_load object-compress
        -force_load object-concurrent
        -force_load object-core
        -force_load object-crypto
        # -force_load object-db
        -force_load object-encoding
        #-force_load object-media
        # -force_load object-message
        -force_load object-net
        # -force_load object-scripts
        # -force_load object-stub
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

macro (add_module_lists)
    SET (module_lists "")
    list(APPEND module_lists "src/argument")
    list(APPEND module_lists "src/mockery")
    list(APPEND module_lists "src/concurrent")
    list(APPEND module_lists "src/core")
    list(APPEND module_lists "src/stub")
    list(APPEND module_lists "src/encoding")
    list(APPEND module_lists "src/net")
    list(APPEND module_lists "src/node")

    list(APPEND module_lists "src/compress")
    list(APPEND module_lists "src/crypto")
    list(APPEND module_lists "src/database")
    # list(APPEND module_lists "src/attacher")
    # list(APPEND module_lists "src/scripts")
    list(APPEND module_lists "src/archive")

    # list(APPEND module_lists "src/drivers")
    # list(APPEND module_lists "3rd/test_process")
    # list(APPEND module_lists "3rd/attacher-builtin")
    # list(APPEND module_lists "3rd/testlib")
    
    #list(APPEND module_lists "src/media")
    #list(APPEND module_lists "src/ui")
    #list(APPEND module_lists "3rd/app")

    list(APPEND module_lists "tests")

    list(APPEND module_lists "src/.")
    add_subdirectories("${module_lists}")
endmacro()

# add compile paramater 
macro (add_compiling_options)
    add_definitions(-DDEBUG)
    add_definitions(-O0 -g)
    add_definitions(-fPIC)
    #add_definitions(-Wl,--no-whole-archive)
    #add_definitions(-Wl,--whole-archive)
    add_definitions(-Wall)
    add_definitions(-Wno-return-type)
    add_definitions(-Wno-unused-variable)
    add_definitions(-Wno-unused-function)
    add_definitions(-Wno-sometimes-uninitialized)
    add_definitions(-Wno-char-subscripts)
    add_definitions(-Wno-sometimes-uninitialized)
    add_definitions(-Wno-unused-label)
    add_definitions(-Wno-uninitialized)
    add_definitions(-Wno-int-conversion)
    add_definitions(-Wno-implicit-function-declaration)
    add_definitions(-Wno-nullability-completeness)
    add_definitions(-Wno-expansion-to-defined)
    add_definitions(-Wno-deprecated-declarations)
    add_definitions(-Wno-pointer-sign)
    add_definitions(-Wno-incompatible-pointer-types)
    add_definitions(-Wno-pointer-to-int-cast)
    add_definitions(-Wno-address-of-packed-member)
endmacro()