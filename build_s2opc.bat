@set CURRENT_DIR=%~dp0
@pushd .
@REM Adapt path to S2OPC root directory if script is not in S2OPC root directory
@REM Note: default paths are set without parenthesized block, as a ")" in the script directory path would end it
@IF NOT DEFINED S2OPC_DIR set "S2OPC_DIR=%CURRENT_DIR%"
@REM Adapt path to extracted check library CMakeLists.txt directory path
@REM Note: check only needed if ENABLE_TESTING is not OFF
@IF NOT DEFINED CHECK_DIR set "CHECK_DIR=%CURRENT_DIR%..\check-0.14.0"
@set CHECK_INSTALL_DIR="%CHECK_DIR%\install"
@IF not %ERRORLEVEL% == 0 exit /B
@
@REM Adapt path to extracted expat library CMakeLists.txt directory path
@IF NOT DEFINED EXPAT_DIR set "EXPAT_DIR=%CURRENT_DIR%..\libexpat-R_2_9_0\expat"
@set EXPAT_INSTALL_DIR="%EXPAT_DIR%\install"
@
@REM Adapt path to extracted mbedtls library CMakeLists.txt directory path
@IF NOT DEFINED MBEDTLS_DIR set "MBEDTLS_DIR=%CURRENT_DIR%..\mbedtls-3.6.7"
@set MBEDTLS_INSTALL_DIR="%MBEDTLS_DIR%\install"
@IF not %ERRORLEVEL% == 0 exit /B
@
@REM Adapt path to extracted paho library CMakeLists.txt directory path
@IF NOT DEFINED PAHO_DIR set "PAHO_DIR=%CURRENT_DIR%..\paho.mqtt.c-1.3.15"
@set PAHO_INSTALL_DIR="%PAHO_DIR%\install"
@IF not %ERRORLEVEL% == 0 exit /B
@
@IF NOT DEFINED S2OPC_NANO_PROFILE (set S2OPC_NANO_PROFILE=off)
@IF NOT DEFINED S2OPC_NODE_MANAGEMENT (set S2OPC_NODE_MANAGEMENT=off)
@IF NOT DEFINED S2OPC_NODE_ADD_OPTIONAL (set S2OPC_NODE_ADD_OPTIONAL=off)
@IF NOT DEFINED S2OPC_NODE_INTERNAL_ADD_CHILD_NODES (set S2OPC_NODE_INTERNAL_ADD_CHILD_NODES=on)
@IF NOT DEFINED S2OPC_NODE_ADD_INVERSE_TYPEDEF (set S2OPC_NODE_ADD_INVERSE_TYPEDEF=on)
@IF NOT DEFINED S2OPC_NODE_DISABLE_CHECK_UNIQUENESS_BROWSENAME (set S2OPC_NODE_DISABLE_CHECK_UNIQUENESS_BROWSENAME=off)
@IF NOT DEFINED S2OPC_NODE_DELETE_CHILD_NODES (set S2OPC_NODE_DELETE_CHILD_NODES=on)
@IF NOT DEFINED S2OPC_NODE_DELETE_ORGANIZES_CHILD_NODES (set S2OPC_NODE_DELETE_ORGANIZES_CHILD_NODES=on)
@IF NOT DEFINED S2OPC_EVENT_MANAGEMENT (set S2OPC_EVENT_MANAGEMENT=off)
@IF NOT DEFINED S2OPC_HAS_AUDITING (set S2OPC_HAS_AUDITING=off)
@IF NOT DEFINED S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE (set S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE=off)
@IF NOT DEFINED S2OPC_CLIENTSERVER_ONLY (set S2OPC_CLIENTSERVER_ONLY=off)
@IF NOT DEFINED S2OPC_PUBSUB_ONLY (set S2OPC_PUBSUB_ONLY=off)
@IF NOT DEFINED WARNINGS_AS_ERRORS (set WARNINGS_AS_ERRORS=on)
@IF NOT DEFINED BUILD_SHARED_LIBS (set BUILD_SHARED_LIBS=on)
@IF NOT DEFINED ENABLE_TESTING (set ENABLE_TESTING=off)
@IF NOT DEFINED ENABLE_SAMPLES (set ENABLE_SAMPLES=off)
@IF NOT DEFINED WITH_ASAN (set WITH_ASAN=off)
@IF NOT DEFINED WITH_PYS2OPC (set WITH_PYS2OPC=off)
@REM REBUILD_DEPS=off skips building a dependency already built by this script (marker file in its install directory)
@IF NOT DEFINED REBUILD_DEPS (set REBUILD_DEPS=on)
@IF NOT DEFINED PYS2OPC_VERSION (set PYS2OPC_VERSION="pys2opc-X.X.X")
@IF NOT DEFINED PYS2OPC_WHEEL_NAME (set PYS2OPC_WHEEL_NAME="%PYS2OPC_VERSION%-cp3XX-cp3XX-win_amd64.whl")
@IF NOT "%ENABLE_TESTING%" == "off" IF NOT "%BUILD_SHARED_LIBS%" == "off" (echo ERROR: cannot build tests ENABLE_TESTING=on with shared libraries BUILD_SHARED_LIBS=on & goto :EXIT_FAIL)
@IF NOT "%ENABLE_TESTING%" == "off" IF NOT "%ENABLE_SAMPLES%" == "on" (echo ERROR: cannot build tests ENABLE_TESTING=on without samples ENABLE_SAMPLES=off & goto :EXIT_FAIL)
@
@IF NOT DEFINED S2OPC_INSTALL set "S2OPC_INSTALL=%S2OPC_DIR%/install"
@
@REM Paths of library providing the CMake export feature
@set CMAKE_LIBS_DIRS=%MBEDTLS_INSTALL_DIR%;%EXPAT_INSTALL_DIR%;%CHECK_INSTALL_DIR%;%PAHO_INSTALL_DIR%
@IF not %ERRORLEVEL% == 0 exit /B
@
@REM VisualStudio version used for compilation: >= VS2017 necessary for S2OPC C99 compatibility
@IF NOT DEFINED VS_VERSION (set VS_VERSION="Visual Studio 18 2026")
@IF NOT DEFINED CONFIG (set CONFIG=Release)
@IF NOT "%WITH_ASAN%" == "off" IF NOT "%CONFIG%" == "Debug" (echo ERROR: cannot compile WITH_ASAN="on" without CONFIG="Debug". & goto :EXIT_FAIL)
@
@REM Comment if ENABLED_TESTING=off or if already built
@REM Build Check
@IF NOT "%ENABLE_TESTING%" == "off" ( call :BUILD_DEP check "%CHECK_DIR%" || goto :EXIT_FAIL )
@
@REM Build Expat (optional: skipped if EXPAT_DIR does not exist, only needed for XML parsing)
@call :BUILD_DEP expat "%EXPAT_DIR%" optional || goto :EXIT_FAIL
@
@REM Comment if already built
@REM Build MbedTLS
@call :BUILD_DEP mbedtls "%MBEDTLS_DIR%" || goto :EXIT_FAIL
@
@REM Build Paho (optional: skipped if PAHO_DIR does not exist, only needed for PubSub MQTT)
@call :BUILD_DEP paho "%PAHO_DIR%" optional || goto :EXIT_FAIL
@
REM Configure S2OPC Project
@cd /D %S2OPC_DIR%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@REM Uncomment to clear build dir at each build
@REM rmdir /S /Q build
@IF NOT DEFINED BUILD_DIR set BUILD_DIR=build
@mkdir %BUILD_DIR%
@cd /D %BUILD_DIR%
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
@
cmake -DWARNINGS_AS_ERRORS=%WARNINGS_AS_ERRORS% -DCMAKE_INSTALL_PREFIX=%S2OPC_INSTALL% -DBUILD_SHARED_LIBS=%BUILD_SHARED_LIBS% -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=%BUILD_SHARED_LIBS% -DS2OPC_CLIENTSERVER_ONLY=%S2OPC_CLIENTSERVER_ONLY% -DS2OPC_PUBSUB_ONLY=%S2OPC_PUBSUB_ONLY% -DWITH_PYS2OPC=%WITH_PYS2OPC% -DPYS2OPC_WHEEL_NAME=%PYS2OPC_WHEEL_NAME% -DS2OPC_NANO_PROFILE=%S2OPC_NANO_PROFILE% -DS2OPC_EVENT_MANAGEMENT=%S2OPC_EVENT_MANAGEMENT% -DS2OPC_HAS_AUDITING=%S2OPC_HAS_AUDITING% -DS2OPC_NODE_MANAGEMENT=%S2OPC_NODE_MANAGEMENT% -DS2OPC_NODE_ADD_OPTIONAL=%S2OPC_NODE_ADD_OPTIONAL% -DS2OPC_NODE_INTERNAL_ADD_CHILD_NODES=%S2OPC_NODE_INTERNAL_ADD_CHILD_NODES% -DS2OPC_NODE_ADD_INVERSE_TYPEDEF=%S2OPC_NODE_ADD_INVERSE_TYPEDEF% -DS2OPC_NODE_DISABLE_CHECK_UNIQUENESS_BROWSENAME=%S2OPC_NODE_DISABLE_CHECK_UNIQUENESS_BROWSENAME% -DS2OPC_NODE_DELETE_CHILD_NODES=%S2OPC_NODE_DELETE_CHILD_NODES% -DS2OPC_NODE_DELETE_ORGANIZES_CHILD_NODES=%S2OPC_NODE_DELETE_ORGANIZES_CHILD_NODES% -DS2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE=%S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE% -DCMAKE_PREFIX_PATH=%CMAKE_LIBS_DIRS% -DENABLE_TESTING=%ENABLE_TESTING% -DENABLE_SAMPLES=%ENABLE_SAMPLES% -DWITH_ASAN=%WITH_ASAN% .. -G %VS_VERSION%
@
@IF not %ERRORLEVEL% == 0 goto :EXIT_FAIL
REM Build S2OPC Project
cmake --build . --config %CONFIG% -j %NUMBER_OF_PROCESSORS%
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

:BUILD_DEP
@REM %1: dependency name (CMake\build_%1.bat), %2: dependency directory,
@REM %3: "optional" if S2OPC can be built without the dependency
@REM Returns 0 if the dependency is built, already built or optional and absent, 1 otherwise
@REM Note: no parenthesized block below, as a ")" in the expanded %~2 path would end it
@IF NOT EXIST "%~2\" IF "%~3" == "optional" goto :BUILD_DEP_ABSENT
@IF "%REBUILD_DEPS%" == "off" IF EXIST "%~2\install\.s2opc_built" goto :BUILD_DEP_ALREADY_BUILT
@REM Remove a previous marker so that a failed rebuild is never considered as built
@IF EXIST "%~2\install\.s2opc_built" del "%~2\install\.s2opc_built"
@call CMake\build_%1.bat
@IF NOT %ERRORLEVEL% == 0 goto :BUILD_DEP_FAILED
@IF NOT EXIST "%~2\install\" goto :BUILD_DEP_NO_INSTALL
@echo built> "%~2\install\.s2opc_built"
@exit /B 0
:BUILD_DEP_ABSENT
@echo %1 not found in %~2, build skipped as this dependency is optional
@exit /B 0
:BUILD_DEP_ALREADY_BUILT
@echo %1 already built in %~2, build skipped
@exit /B 0
:BUILD_DEP_FAILED
@echo ERROR: build of %1 failed
@exit /B 1
:BUILD_DEP_NO_INSTALL
@echo ERROR: build of %1 did not produce %~2\install
@exit /B 1
