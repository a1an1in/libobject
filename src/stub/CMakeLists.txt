project(object-stub)

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
if("${PLATFORM}" STREQUAL "android")
    target_link_libraries(${PROJECT_NAME} object-core log)
else()
    target_link_libraries(${PROJECT_NAME} object-core)
endif()
