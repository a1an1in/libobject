project(dl)

file(GLOB_RECURSE MOUDLE_HEADERS
    "*.h"
)

file(GLOB_RECURSE MOUDLE_SOURCES
    "*.c"
)

message(STATUS "Include directories for windows-dl: ${CMAKE_INCLUDE_PATH}")

file(GLOB MOUDLE_SRC
    ${MOUDLE_HEADERS}
    ${MOUDLE_SOURCES}
)

# The dlfcn.h replacement has to be in the include path before anything is
# compiled, so copy it at configure time (independent of the make order).
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)
file(COPY ${PROJECT_SOURCE_DIR}/src/dlfcn.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

add_library(${PROJECT_NAME} SHARED ${MOUDLE_SRC})

# Put the DLL next to the executables so that it is found at runtime.
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/dlfcn.h ${CMAKE_INSTALL_PREFIX}/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/bin
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_INSTALL_PREFIX}/bin
    COMMENT "Installing windows-dl inlcude file"
)
