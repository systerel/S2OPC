@REM Uncomment or set variables to build without build_s2opc script
@REM set CHECK_DIR=%~dp0..\check-0.14.0
@REM set CHECK_INSTALL_DIR="%CHECK_DIR%\install"
@REM set VS_VERSION="Visual Studio 15 2017 Win64"
@REM set CONFIG="Release"
@
@set CHECK_BUILD_DIR="%CHECK_DIR%\build"
@
REM Build CHECK
@cd %CHECK_DIR%
@mkdir %CHECK_BUILD_DIR%
@cd %CHECK_BUILD_DIR%
@REM Change DCHECK_SHARED_LIBS to ON in order to use DLL instead of static library
cmake -DCHECK_SHARED_LIBS=Off -DCHECK_BUILD_TOOLS=Off -DCHECK_BUILD_EXAMPLES=Off -DCHECK_BUILD_TESTS=Off -DCMAKE_INSTALL_PREFIX=%CHECK_INSTALL_DIR% .. -G %VS_VERSION%
cmake --build . --target install --config %CONFIG%
