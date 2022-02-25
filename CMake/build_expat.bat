@REM Uncomment or set variables to build without build_s2opc script
@REM set EXPAT_DIR=%~dp0..\libexpat-R_2_4_3\expat
@REM set EXPAT_INSTALL_DIR="%EXPAT_DIR%\install"
@REM set VS_VERSION="Visual Studio 15 2017 Win64"
@REM set CONFIG="Release"
@
@set EXPAT_BUILD_DIR="%EXPAT_DIR%\build"
@
REM Build Expat
@cd %EXPAT_DIR%
@rmdir /S /Q %EXPAT_BUILD_DIR%
@mkdir %EXPAT_BUILD_DIR%
@cd %EXPAT_BUILD_DIR%
@REM Change DEXPAT_SHARED_LIBS to ON in order to use DLL instead of static library
cmake -DEXPAT_SHARED_LIBS=Off -DEXPAT_BUILD_TOOLS=Off -DEXPAT_BUILD_EXAMPLES=Off -DEXPAT_BUILD_TESTS=Off -DCMAKE_INSTALL_PREFIX=%EXPAT_INSTALL_DIR% .. -G %VS_VERSION%
cmake --build . --target install --config %CONFIG%
