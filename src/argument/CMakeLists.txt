project(object-argument)

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
    # Application.c uses the fshell singleton, which lives in object-scripts.
    target_link_libraries(${PROJECT_NAME} object-scripts object-stub object-concurrent object-core)
else()
    add_library(${PROJECT_NAME} STATIC ${MOUDLE_SRC})
endif()
