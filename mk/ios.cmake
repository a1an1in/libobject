LINK_DIRECTORIES(/usr/local/lib 
                 /usr/lib
                 ${PROJECT_SOURCE_DIR}/lib)

INCLUDE_DIRECTORIES(/usr/local/include
                    ${PROJECT_SOURCE_DIR}/src/include)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/ios)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/ios)
