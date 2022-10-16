include(ExternalProject)

if(EXISTS ${PROJECT_SOURCE_DIR}/3rd/libobject)
    message("rebuild libobject")
    ExternalProject_Add(libobject
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/libobject
        CMAKE_ARGS ${BUILD_EXTERNAL_ARGS})
else ()
    message("git clone and build libobject")
    ExternalProject_Add(libobject
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/libobject
        GIT_REPOSITORY git@github.com:a1an1in/libobject.git
        CMAKE_ARGS ${BUILD_EXTERNAL_ARGS})
endif()

#ExternalProject_Add(libobject
    #SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/libobject
    ##GIT_REPOSITORY git@github.com:a1an1in/libobject.git
    #BUILD_COMMAND cd "${PROJECT_SOURCE_DIR}/3rd/libobject" && pwd && "./build_mac.sh"
    #INSTALL_COMMAND ""
    #)
