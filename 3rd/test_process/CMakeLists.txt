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
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-s")
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-s)

if("${PLATFORM}" STREQUAL "android")
    target_link_libraries(${PROJECT_NAME} dl)
else()
    target_link_libraries(${PROJECT_NAME} pthread)
endif()


