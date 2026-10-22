project(object-scripts)

file(GLOB_RECURSE MOUDLE_HEADERS
    "*.h"
)

file(GLOB_RECURSE MOUDLE_SOURCES
    "*.c"
)

file(GLOB MOUDLE_SRC
    ${MOUDLE_HEADERS}
    ${MOUDLE_SOURCES}
)

if ("${PLATFORM}" STREQUAL "windows")
    add_library(${PROJECT_NAME} SHARED ${MOUDLE_SRC})
    add_dependencies(${PROJECT_NAME} object-core)
    # NOTE: object-argument links object-scripts (it uses the fshell
    # singleton), so object-argument must not be linked here, otherwise the
    # two shared libraries would import from each other.
    target_link_libraries(${PROJECT_NAME} object-concurrent object-stub object-core dl)
else()
    add_library(${PROJECT_NAME} STATIC ${MOUDLE_SRC})
endif()