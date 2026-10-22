project(object-archive)

# The archive formats need the compression libraries:
#   7z    -> liblzma + zlib
#   squashfs -> zlib
#   tbz2  -> libbz2
find_package(BZip2)
find_package(LibLZMA)
find_package(ZLIB)

if (NOT BZIP2_FOUND OR NOT LIBLZMA_FOUND OR NOT ZLIB_FOUND)
    message(FATAL_ERROR
        "The archive module needs bzip2, lzma and zlib development files.\n"
        "  Debian/Ubuntu : sudo apt-get install libbz2-dev liblzma-dev zlib1g-dev\n"
        "  MSYS2/MinGW   : pacman -S mingw-w64-x86_64-bzip2 mingw-w64-x86_64-xz mingw-w64-x86_64-zlib")
endif ()

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

add_library(${PROJECT_NAME} STATIC ${MOUDLE_SRC})

target_link_libraries(${PROJECT_NAME} ${BZIP2_LIBRARIES} ${LIBLZMA_LIBRARIES} ${ZLIB_LIBRARIES})
