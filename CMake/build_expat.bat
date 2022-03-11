@REM Uncomment or set variables to build without build_s2opc script
@REM set EXPAT_DIR=%~dp0..\libexpat-R_2_4_3\expat
@REM set EXPAT_INSTALL_DIR="%EXPAT_DIR%\install"
@REM set VS_VERSION="Visual Studio 15 2017 Win64"
@REM set CONFIG="Release"
@
REM Build Expat
@pushd .
@set EXPAT_BUILD_DIR="%EXPAT_DIR%\build"
@IF not %ERRORLEVEL% == 0 exit /B
@
@cd /D %EXPAT_DIR%
@IF not %ERRORLEVEL% == 0 exit /B
@mkdir %EXPAT_BUILD_DIR%
@cd /D %EXPAT_BUILD_DIR%
@IF not %ERRORLEVEL% == 0 exit /B
@REM Change DEXPAT_SHARED_LIBS to ON in order to use DLL instead of static library
cmake -DEXPAT_SHARED_LIBS=Off -DEXPAT_BUILD_TOOLS=Off -DEXPAT_BUILD_EXAMPLES=Off -DEXPAT_BUILD_TESTS=Off -DCMAKE_INSTALL_PREFIX=%EXPAT_INSTALL_DIR% .. -G %VS_VERSION%
@IF not %ERRORLEVEL% == 0 exit /B
cmake --build . --target install --config %CONFIG%
@IF not %ERRORLEVEL% == 0 exit /B
@popd
