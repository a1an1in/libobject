LINK_DIRECTORIES(
    C:/mingw64/lib
    C:/mingw64/x86_64-w64-mingw32/lib
    "C:/Program Files (x86)/Microsoft SDKs/Windows Kits/10/ExtensionSDKs/Microsoft.UniversalCRT.Debug/10.0.19041.0/Redist/Debug/x64"
    ${CMAKE_INSTALL_PREFIX}/lib
    PARENT_SCOPE)

INCLUDE_DIRECTORIES(
    C:/mingw64/include
    C:/mingw64/x86_64-w64-mingw32/include
    ${PROJECT_SOURCE_DIR}/src/include
    ${CMAKE_INSTALL_PREFIX}/include)