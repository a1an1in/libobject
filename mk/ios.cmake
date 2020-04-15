macro (set_cmake_evironment_variable)
    if (IOS_PLATFORM STREQUAL "OS")
        if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
            set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/ios)
        endif ()
        LINK_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/lib
            ${PROJECT_SOURCE_DIR}/lib/ios)
        INCLUDE_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/include
            ${PROJECT_SOURCE_DIR}/src/include)
        set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
        set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
        #set (CMAKE_INSTALL_PREFIX /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr)
    elseif (IOS_PLATFORM STREQUAL "SIMULATOR64")
        if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
            set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/SIMULATOR64)
        endif ()
        LINK_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/lib
            ${CMAKE_INSTALL_PREFIX}/lib)
        INCLUDE_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include
            ${CMAKE_INSTALL_PREFIX}/include
            ${PROJECT_SOURCE_DIR}/src/include)
        set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
        set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)

        message("aaaaaa CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
        #set (CMAKE_INSTALL_PREFIX /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr)
        #SET(ExternalLibs ${ExternalLibs} avformat avfilter swscale swresample avcodec avutil x264 vorbis vorbisenc
            #vorbisfile mp3lame vpx xvidcore opus fdk-aac theora xvidcore z iconv bz2 SDL2 SDL2_ttf yuv -force_load /usr/local/lib/libobject.a)
        #SET(ExternalLibs ${ExternalLibs} avformat avfilter swscale swresample avcodec avutil SDL2 SDL2_ttf)
        #SET(ExternalLibs ${ExternalLibs} z iconv bz2)
        #SET(ExternalLibs ${ExternalLibs} "-force_load /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/lib/libobject.a")
        set (SYS_LIBRARY_PATH /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/lib )

    else()
        message(FATAL_ERROR "Invalid IOS_PLATFORM: ${IOS_PLATFORM}")
    endif()

    find_library(Security_Path Security)
    if (NOT Security_Path)
        message(FATAL_ERROR "Security not found")
    endif()

    find_library(CoreMedia_Path CoreMedia)
    if (NOT CoreMedia_Path)
        message(FATAL_ERROR "CoreMedia not found")
    endif()

    find_library(CoreVideo_Path CoreVideo)
    if (NOT CoreVideo_Path)
        message(FATAL_ERROR "CoreMedia not found")
    else ()
        message("CoreVideo_Path: ${CoreVideo_Path}")
    endif()


    find_library(VideoToolBox_Path VideoToolBox)
    if (NOT VideoToolBox_Path)
        message(FATAL_ERROR "VideoToolBox not found")
    endif()

    find_library(Foundation_Path Foundation)
    if (NOT Foundation_Path)
        message(FATAL_ERROR "Foundation_Path not found")
    else ()
        message("Foundation_Path: ${Foundation_Path}")
    endif()

    find_library(AVFoundation_Path AVFoundation)
    if (NOT AVFoundation_Path)
        message(FATAL_ERROR "Foundation_Path not found")
    else ()
        message("AVFoundation_Path: ${AVFoundation_Path}")
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

    find_library(CoreMotion_Path CoreMotion)
    if (NOT CoreMotion_Path)
        message(FATAL_ERROR "CoreMotion_Path not found")
    else ()
        message("CoreMotion_Path: ${CoreMotion_Path}")
    endif()

    #find_library(OpenGLES_Path OpenGLES)
    #if (NOT OpenGLES_Path)
        #message(FATAL_ERROR "OpenGLES_Path not found")
    #else ()
        #message("OpenGLES_Path: ${OpenGLES_Path}")
    #endif()

    #find_library(UIKIT_Path UIKIT)
    #if (NOT UIKIT_Path)
        #message(FATAL_ERROR "UIKIT_Path not found")
    #else ()
        #message("UIKIT_Path: ${UIKIT_Path}")
    #endif()

    find_library(GameController_Path GameController)
    if (NOT GameController_Path)
        message(FATAL_ERROR "GameController_Path not found")
    else ()
        message("GameController_Path: ${GameController_Path}")
    endif()

    find_library(CoreAudio_Path CoreAudio)
    if (NOT CoreAudio_Path)
        message(FATAL_ERROR "CoreAudio_Path not found")
    else ()
        message("CoreAudio_Path: ${CoreAudio_Path}")
    endif()

    find_library(CoreBluetooth_Path CoreBluetooth)
    if (NOT CoreBluetooth_Path)
        message(FATAL_ERROR "CoreBluetooth_Path not found")
    else ()
        message("CoreBluetooth_Path: ${CoreBluetooth_Path}")
    endif()

    #SET(ExternalLibs ${ExternalLibs} ${CoreMedia_Path} ${VideoToolBox_Path} ${Security_Path}
        #${Foundation_Path} ${AudioToolbox_Path} ${CoreGraphics_Path} ${QuartzCore_Path} ${OpenAL_Path} 
        #${AVFoundation_Path} ${CoreVideo_Path} ${CoreMotion_Path} ${mediaToolbox_Path} 
        #${OpenGLES_Path} ${UIKIT_Path} ${GameController_Path} ${CoreAudio_Path}
        #${CoreBluetooth_Path})
    SET(ExternalLibs ${ExternalLibs} ${CoreMedia_Path} ${VideoToolBox_Path} ${Security_Path}
        ${Foundation_Path} ${AudioToolbox_Path} ${CoreGraphics_Path} ${QuartzCore_Path} ${OpenAL_Path} 
        ${AVFoundation_Path} ${CoreVideo_Path} ${CoreMotion_Path} ${mediaToolbox_Path} 
        ${GameController_Path} ${CoreAudio_Path}
        ${CoreBluetooth_Path})

endmacro()

macro (display_ios_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
