project(attacher-builtin)

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
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-s)   
target_link_libraries(${PROJECT_NAME} dl)