# ---------------------------------------------------------------------------
# MinGW-w64 toolchain for native Windows builds.
#
# The MinGW-w64 root is auto detected, the following locations are tried
# in order:
#   1. -DMINGW_ROOT=<path>
#   2. C:/mingw64            (classic MinGW-w64 installer layout)
#   3. C:/msys64/mingw64     (MSYS2 MINGW64 environment)
#   4. C:/msys64/ucrt64      (MSYS2 UCRT64 environment)
# ---------------------------------------------------------------------------

if (NOT MINGW_ROOT)
    foreach (_mingw_candidate
             "C:/mingw64"
             "C:/msys64/mingw64"
             "C:/msys64/ucrt64"
             "C:/msys64/mingw32")
        if (EXISTS "${_mingw_candidate}/bin/gcc.exe")
            set(MINGW_ROOT "${_mingw_candidate}")
            break()
        endif()
    endforeach()
endif()

if (NOT MINGW_ROOT)
    set(MINGW_ROOT "C:/mingw64")
endif()

message(STATUS "MinGW-w64 root: ${MINGW_ROOT}")

# Use the native (Windows) compilers of the MinGW-w64 environment.
if (EXISTS "${MINGW_ROOT}/bin/gcc.exe")
    set(CMAKE_C_COMPILER "${MINGW_ROOT}/bin/gcc.exe")
    set(CMAKE_CXX_COMPILER "${MINGW_ROOT}/bin/g++.exe")
    set(CMAKE_RC_COMPILER "${MINGW_ROOT}/bin/windres.exe")
endif()

# Let find_package()/find_library() locate the libraries shipped with MinGW-w64
# (zlib, bzip2, lzma, ...).
set(CMAKE_PREFIX_PATH "${MINGW_ROOT}" $ENV{CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH "${MINGW_ROOT}")

LINK_DIRECTORIES(
    ${MINGW_ROOT}/lib
    ${MINGW_ROOT}/x86_64-w64-mingw32/lib
    ${CMAKE_INSTALL_PREFIX}/lib
    )

INCLUDE_DIRECTORIES(
    ${MINGW_ROOT}/include
    ${MINGW_ROOT}/x86_64-w64-mingw32/include
    ${PROJECT_SOURCE_DIR}/src/include
    ${CMAKE_INSTALL_PREFIX}/include)
