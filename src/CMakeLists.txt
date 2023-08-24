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
target_link_libraries(${PROJECT_NAME} ${ExternalLibs}  -Wl,--no-as-needed -lobject-testlib2 -Wl,--as-needed object-testlib)
#add_dependencies(${PROJECT_NAME} object)

#install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${LIBRARY_DIRECTORY})
