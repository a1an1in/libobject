project(regex)

# 定义一个自定义目标，仅用于拷贝文件
add_custom_target(copy_regex_files ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/regex.h ${CMAKE_INSTALL_PREFIX}/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/lib
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/libregex.dll ${CMAKE_INSTALL_PREFIX}/lib
    COMMENT "Copying regex files to the installation directory"
)