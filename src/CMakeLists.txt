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
if ("${PLATFORM}" STREQUAL "windows")
    target_link_libraries(${PROJECT_NAME} ${ExternalLibs})
elseif("${PLATFORM}" STREQUAL "mac")
    target_link_libraries(${PROJECT_NAME} ${ExternalLibs})
else()
    target_link_libraries(${PROJECT_NAME} ${ExternalLibs}  -Wl,--as-needed object-testlib2 -Wl,--as-needed object-testlib)
endif()
#add_dependencies(${PROJECT_NAME} object)

#install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${LIBRARY_DIRECTORY})
