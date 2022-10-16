project(xtools)

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
target_link_libraries(${PROJECT_NAME} ${ExternalLibs})
target_link_libraries(${PROJECT_NAME} mysqlclient)
#add_dependencies(${PROJECT_NAME} object)

#set_compiler_flags(${PROJECT_NAME})

#target_include_directories(${PROJECT_NAME} PUBLIC "${YHSDK_INCLUDE_DIR}")
#target_link_libraries(${PROJECT_NAME} yhnet)

#install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${LIBRARY_DIRECTORY})
