project(object-net)

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

#add_library(${PROJECT_NAME} SHARED ${MOUDLE_SRC})
add_library(${PROJECT_NAME} STATIC ${MOUDLE_SRC})
#target_link_libraries(${PROJECT_NAME} object)
#add_dependencies(${PROJECT_NAME} object)

#set_compiler_flags(${PROJECT_NAME})

#target_include_directories(${PROJECT_NAME} PUBLIC "${YHSDK_INCLUDE_DIR}")
#target_link_libraries(${PROJECT_NAME} yhnet)

#install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${LIBRARY_DIRECTORY})
