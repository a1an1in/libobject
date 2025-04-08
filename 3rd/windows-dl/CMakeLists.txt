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

add_library(${PROJECT_NAME} SHARED ${MOUDLE_SRC})

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/dlfcn.h ${CMAKE_INSTALL_PREFIX}/include
    COMMENT "Installing windows-dl inlcude file"
)