@set CURRENT_DIR=%~dp0
@pushd .
@REM Adapt path to S2OPC root directory if script is not in S2OPC root directory
@IF NOT DEFINED S2OPC_DIR (set S2OPC_DIR=%CURRENT_DIR%)
@REM Adapt path to extracted check library CMakeLists.txt directory path
@REM Note: check only needed if ENABLE_TESTING is not OFF
@IF NOT DEFINED CHECK_DIR (set CHECK_DIR=%CURRENT_DIR%..\check-0.14.0)
@set CHECK_INSTALL_DIR="%CHECK_DIR%\install"
@IF not %ERRORLEVEL% == 0 exit /B
@
@REM Adapt path to extracted expat library CMakeLists.txt directory path
@IF NOT DEFINED EXPAT_DIR (set EXPAT_DIR=%CURRENT_DIR%..\expat-2.7.2)
@set EXPAT_INSTALL_DIR="%EXPAT_DIR%\install"
@
@REM Adapt path to extracted mbedtls library CMakeLists.txt directory path
@IF NOT DEFINED MBEDTLS_DIR (set MBEDTLS_DIR=%CURRENT_DIR%..\mbedtls-3.6.4)
@set MBEDTLS_INSTALL_DIR="%MBEDTLS_DIR%\install"
@IF not %ERRORLEVEL% == 0 exit /B
@
@IF NOT DEFINED S2OPC_NANO_PROFILE (set S2OPC_NANO_PROFILE=off)
@IF NOT DEFINED S2OPC_EVENT_MANAGEMENT (set S2OPC_EVENT_MANAGEMENT=off)
@IF NOT DEFINED S2OPC_NODE_MANAGEMENT (set S2OPC_NODE_MANAGEMENT=off)
@IF NOT DEFINED S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE (set S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE=off)
@IF NOT DEFINED BUILD_SHARED_LIBS (set BUILD_SHARED_LIBS=on)
@IF NOT DEFINED ENABLE_TESTING (set ENABLE_TESTING=off)
@IF NOT DEFINED ENABLE_SAMPLES (set ENABLE_SAMPLES=off)
@IF NOT DEFINED WITH_PYS2OPC (set WITH_PYS2OPC=off)
@IF NOT DEFINED PYS2OPC_WHEEL_NAME (set PYS2OPC_WHEEL_NAME="")
@IF NOT "%ENABLE_TESTING%" == "off" IF NOT "%BUILD_SHARED_LIBS%" == "off" (echo ERROR: cannot build tests ENABLE_TESTING=1 with shared libraries BUILD_SHARED_LIBS=1 & goto :EXIT_FAIL)
@
@IF NOT DEFINED S2OPC_INSTALL (set S2OPC_INSTALL=%S2OPC_DIR%/install)
@
@REM Paths of library providing the CMake export feature
@set CMAKE_LIBS_DIRS=%MBEDTLS_INSTALL_DIR%;%EXPAT_INSTALL_DIR%;%CHECK_INSTALL_DIR%
@IF not %ERRORLEVEL% == 0 exit /B
@
@REM VisualStudio version used for compilation: >= VS2017 necessary for S2OPC C99 compatibility
@IF NOT DEFINED VS_VERSION (set VS_VERSION="Visual Studio 17 2022")
@IF NOT DEFINED CONFIG (set CONFIG="Release")
@
@REM Comment if ENABLED_TESTING=off or if already built
@REM Build Check
@IF NOT "%ENABLE_TESTING%" == "off" (@call CMake\build_check.bat)
@
@REM Comment if XML parsing is not needed or if already built
@REM Build Expat
@call CMake\build_expat.bat
@
@REM Comment if already built
@REM Build MbedTLS
@call CMake\build_mbedtls.bat
@
REM Configure S2OPC Project
@cd /D %S2OPC_DIR%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@REM Uncomment to clear build dir at each build
@REM rmdir /S /Q build
@mkdir build
@cd /D build
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@
cmake -DCMAKE_INSTALL_PREFIX=%S2OPC_INSTALL% -DBUILD_SHARED_LIBS=%BUILD_SHARED_LIBS% -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=%BUILD_SHARED_LIBS% -DS2OPC_CLIENTSERVER_ONLY=on -DWITH_PYS2OPC=%WITH_PYS2OPC% -DPYS2OPC_WHEEL_NAME=%PYS2OPC_WHEEL_NAME% -DS2OPC_NANO_PROFILE=%S2OPC_NANO_PROFILE% -DS2OPC_EVENT_MANAGEMENT=%S2OPC_EVENT_MANAGEMENT% -DS2OPC_NODE_MANAGEMENT=%S2OPC_NODE_MANAGEMENT% -DS2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE=%S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE% -DCMAKE_PREFIX_PATH=%CMAKE_LIBS_DIRS% -DENABLE_TESTING=%ENABLE_TESTING% -DENABLE_SAMPLES=%ENABLE_SAMPLES% .. -G %VS_VERSION%
@
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
REM Build S2OPC Project
cmake --build . --config %CONFIG%
cmake --install .
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
:EXIT_OK
@echo S2OPC build successful
@popd
@exit /B 0
:EXIT_FAIL
@echo S2OPC build failed
@popd
@exit /B 1
