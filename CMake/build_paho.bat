@REM Uncomment or set variables to build without build_s2opc script
@REM set PAHO_DIR=%~dp0..\paho.mqtt.c-1.3.15
@REM set PAHO_INSTALL_DIR="%PAHO_DIR%\install"
@REM set VS_VERSION="Visual Studio 15 2017 Win64"
@REM set CONFIG="Release"
@
REM Build Paho
@pushd .
@set PAHO_BUILD_DIR="%PAHO_DIR%\build"
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@
@cd /D %PAHO_DIR%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@mkdir %PAHO_BUILD_DIR%
@cd /D %PAHO_BUILD_DIR%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@REM Change PAHO_BUILD_SHARED to ON and DPAHO_BUILD_STATIC to OFF in order to use DLL instead of static library
cmake -DPAHO_BUILD_SHARED=off -DPAHO_BUILD_STATIC=on -DPAHO_ENABLE_TESTING=off -DCMAKE_INSTALL_PREFIX=%PAHO_INSTALL_DIR% .. -G %VS_VERSION%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
cmake --build . --target install --config %CONFIG%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
:EXIT_OK
@echo Paho library build successful
@popd
@exit /B 0
:EXIT_FAIL
@echo Paho library build failed
@popd
@exit /B 1
