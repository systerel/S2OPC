@REM Uncomment to use without build_s2opc
@REM set MBEDTLS_DIR=%~dp0..\mbedtls-2.16.12
@REM set VS_VERSION="Visual Studio 15 2017 Win64"
@REM set CONFIG="Release"
@
REM Build MbedTLS
@pushd .
@cd /D %MBEDTLS_DIR%
@IF not %ERRORLEVEL% == 0 exit /B
@mkdir build
@cd /D build
@IF not %ERRORLEVEL% == 0 exit /B
@REM Building the shared with export all is not functional for now see (https://github.com/ARMmbed/mbedtls/issues/470)
cmake -DUSE_STATIC_MBEDTLS_LIBRARY=On -DENABLE_PROGRAMS=Off -DENABLE_TESTING=Off -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=Off -DUSE_SHARED_MBEDTLS_LIBRARY=Off .. -G %VS_VERSION%
@IF not %ERRORLEVEL% == 0 exit /B
cmake --build . --target ALL_BUILD --config %CONFIG%
@IF not %ERRORLEVEL% == 0 exit /B
@popd
