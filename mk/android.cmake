LINK_DIRECTORIES(/usr/local/lib 
                 /usr/lib
                 ${PROJECT_SOURCE_DIR}/lib/android)

INCLUDE_DIRECTORIES(/usr/local/include
                    ${PROJECT_SOURCE_DIR}/src/include)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/android/${ANDROID_ABI} PARENT_SCOPE)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/andorid/${ANDROID_ABI} PARENT_SCOPE)
