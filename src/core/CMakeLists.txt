project(object-core)

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

add_library(${PROJECT_NAME} SHARED ${MOUDLE_SRC})
if ("${PLATFORM}" STREQUAL "windows")
    target_link_libraries(${PROJECT_NAME} ws2_32 wsock32 regex pthread)
endif()
