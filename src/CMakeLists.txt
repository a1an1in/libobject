project(object)
include (../mk/util.cmake)

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

find_main_file(Main_File)
list(REMOVE_ITEM MOUDLE_SRC ${Main_File})

add_library(${PROJECT_NAME} STATIC ${MOUDLE_SRC})

add_executable(xtools main.c)
target_link_libraries(xtools ${ExternalLibs})
ADD_DEPENDENCIES(xtools object)
