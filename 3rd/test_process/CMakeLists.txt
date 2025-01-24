project(test-process)

file(GLOB MOUDLE_HEADERS
    "*.h"
)

file(GLOB MOUDLE_SOURCES
    "*.c"
)

file(GLOB MOUDLE_SRC
    ${MOUDLE_HEADERS}
    ${MOUDLE_SOURCES}
)

add_executable(${PROJECT_NAME} ${MOUDLE_SRC})
target_link_libraries(${PROJECT_NAME} -Wl,--no-as-needed -lattacher-builtin -Wl,--as-needed dl pthread)
