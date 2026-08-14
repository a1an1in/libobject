project(object-compress)

# libbz2 is required by Bz2Compress (bzip2). Detect the dev package up front
# and give a clear message when it is missing, instead of failing later on a
# cryptic "bzlib.h: No such file or directory" compiler error.
find_package(BZip2)
if (NOT BZIP2_FOUND)
    message(FATAL_ERROR
        "libbz2 (bzip2) development files were not found.\n"
        "Bz2Compress/Tbz2 need libbz2 to compile and link.\n"
        "  Debian/Ubuntu : sudo apt-get install libbz2-dev\n"
        "  RHEL/Fedora   : sudo dnf install bzip2-devel\n"
        "  macOS         : bzip2 is usually shipped with the OS (brew install bzip2 if missing)")
endif ()

# liblzma (xz) is required by LzmaCompress and SevenZip (7z). Detect the dev
# package up front and give a clear message when it is missing.
find_package(LibLZMA)
if (NOT LIBLZMA_FOUND)
    message(FATAL_ERROR
        "liblzma (LZMA/LZMA2, xz) development files were not found.\n"
        "LzmaCompress/SevenZip need liblzma to compile and link.\n"
        "  Debian/Ubuntu : sudo apt-get install liblzma-dev\n"
        "  RHEL/Fedora   : sudo dnf install xz-devel\n"
        "  macOS         : brew install xz")
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
