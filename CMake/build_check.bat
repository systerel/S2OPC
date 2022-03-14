@REM Uncomment or set variables to build without build_s2opc script
@REM set CHECK_DIR=%~dp0..\check-0.14.0
@REM set CHECK_INSTALL_DIR="%CHECK_DIR%\install"
@REM set VS_VERSION="Visual Studio 15 2017 Win64"
@REM set CONFIG="Release"
@
REM Build CHECK
@pushd .
@set CHECK_BUILD_DIR="%CHECK_DIR%\build"
@
@cd /D %CHECK_DIR%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@mkdir %CHECK_BUILD_DIR%
@cd /D %CHECK_BUILD_DIR%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@REM Change DCHECK_SHARED_LIBS to ON in order to use DLL instead of static library
cmake -DCHECK_SHARED_LIBS=Off -DCHECK_BUILD_TOOLS=Off -DCHECK_BUILD_EXAMPLES=Off -DCHECK_BUILD_TESTS=Off -DCMAKE_INSTALL_PREFIX=%CHECK_INSTALL_DIR% .. -G %VS_VERSION%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
cmake --build . --target install --config %CONFIG%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
:EXIT_OK
@echo Check library build successful
@popd
@exit /B 0
:EXIT_FAIL
@echo Check library build failed
@popd
@exit /B 1
