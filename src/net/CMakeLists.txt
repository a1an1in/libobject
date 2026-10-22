project(object-net)

file(GLOB_RECURSE MOUDLE_HEADERS
    "*.h"
)

file(GLOB_RECURSE MOUDLE_SOURCES
    "*.c"
)

# Android 暂不支持 VPN（TUN 实现依赖 Linux 接口）
if(PLATFORM STREQUAL "android")
    message(STATUS "Android platform detected. Skipping vpn sources.")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/vpn/.*")
endif()

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
