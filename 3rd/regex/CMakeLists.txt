project(regex)

# ---------------------------------------------------------------------------
# The regex implementation is shipped as a prebuilt Windows DLL (libregex.dll)
# plus its header (regex.h).
#
# Two things have to be provided for the Windows build:
#   1. regex.h must be in the include path, otherwise the modules that use
#      <regex.h> fail to compile.
#   2. the MinGW linker needs an *import library* (libregex.dll.a) to resolve
#      "-lregex"; the shipped DLL alone is not enough.
#
# The copies are done at configure time so that they do not depend on the
# order in which the make targets are executed (parallel make).
# ---------------------------------------------------------------------------

file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)

file(COPY ${PROJECT_SOURCE_DIR}/regex.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
file(COPY ${PROJECT_SOURCE_DIR}/libregex.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
file(COPY ${PROJECT_SOURCE_DIR}/libregex.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Generate the import library from the DLL (gendef + dlltool come with
# mingw-w64-tools).
if (WIN32)
    find_program(GENDEF_EXECUTABLE NAMES gendef)
    find_program(DLLTOOL_EXECUTABLE NAMES dlltool)

    if (GENDEF_EXECUTABLE AND DLLTOOL_EXECUTABLE)
        set(REGEX_DEF_FILE ${CMAKE_CURRENT_BINARY_DIR}/libregex.def)
        set(REGEX_IMPORT_LIB ${CMAKE_INSTALL_PREFIX}/lib/libregex.dll.a)

        add_custom_command(
            OUTPUT ${REGEX_IMPORT_LIB}
            COMMAND ${GENDEF_EXECUTABLE} ${PROJECT_SOURCE_DIR}/libregex.dll -o ${REGEX_DEF_FILE}
            COMMAND ${DLLTOOL_EXECUTABLE} -d ${REGEX_DEF_FILE} -l ${REGEX_IMPORT_LIB}
            DEPENDS ${PROJECT_SOURCE_DIR}/libregex.dll
            COMMENT "Generating regex import library: ${REGEX_IMPORT_LIB}"
            VERBATIM
        )

        add_custom_target(regex_import_lib ALL DEPENDS ${REGEX_IMPORT_LIB})
    else ()
        message(WARNING "gendef/dlltool not found, -lregex will not be resolvable. "
                        "Install the mingw-w64-tools package.")
    endif ()
endif ()

# Keep the original build time copy target for compatibility.
add_custom_target(copy_regex_files ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/include
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/regex.h ${CMAKE_INSTALL_PREFIX}/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/lib
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/libregex.dll ${CMAKE_INSTALL_PREFIX}/lib
    COMMENT "Copying regex files to the installation directory"
)
