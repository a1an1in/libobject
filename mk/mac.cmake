LINK_DIRECTORIES(/usr/local/lib 
                 /usr/lib
                 ${PROJECT_SOURCE_DIR}/lib)

INCLUDE_DIRECTORIES(/usr/local/include
                    /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include
                    ${PROJECT_SOURCE_DIR}/src/include)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/mac PARENT_SCOPE)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/mac PARENT_SCOPE)
