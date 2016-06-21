REM Set the MinGW environment variables.
if "%MINGW_BIN%"=="" mingwenv.bat

REM Delete the CMake cache. Needed when changes on the CMakeLists should be applied in a consistent way.
del CMakeCache.txt
rmdir /s /q CMakeFiles

REM Configure using the MinGW compiler chain.
cmake -D"CMAKE_SYSTEM_PREFIX_PATH:PATH=C:\MinGW64\mingw64\x86_64-w64-mingw32" -G "MinGW Makefiles" ..
