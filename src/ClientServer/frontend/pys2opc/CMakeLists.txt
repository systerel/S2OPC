cmake_minimum_required(VERSION 3.12)
project (S2OPC_PyS2OPC)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../../../CommonDefs.cmake)
endif()

##                                                              ##
# PyS2OPC library: Python 3 binding for S2OPC client and servers #
##                                                              ##

# pip lacks the ability to produce the name of the output library before it is done,
#  so it is hardcoded.
set(PYS2OPC_WHEEL_NAME "pys2opc-1.7.0-cp313-cp313-linux_x86_64.whl" CACHE STRING
    "The guessed name of the produced Python 3 Wheel. It must be set to the correct name of the wheel that will be produced in build/lib for tests to run.")

if(WITH_PYS2OPC)

  ### Dependencies ###

  # EXPAT dependency managed in CommonDefs
  # use find_package onto s2opc in case we build s2opc and PyS2OPC separately
  find_package(s2opc)
  # Python3 is mandatory for compilation
  # To get PYTHON_INCLUDE_DIRS needed for `#include "Python.h"`
  find_package(Python3 COMPONENTS Development Interpreter REQUIRED)

  # Check requirements : autopxd + cython
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import autopxd"
    RESULT_VARIABLE EXIT_CODE_AUTOPXD
    OUTPUT_QUIET
  )
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import cython"
    RESULT_VARIABLE EXIT_CODE_CYTHON
    OUTPUT_QUIET
  )
  if (NOT ${EXIT_CODE_AUTOPXD} EQUAL 0)
    message(FATAL_ERROR
            "The \"autopxd\" Python3 package is not installed. Please install it using the following command: \"pip install autopxd2\"."
           )
  endif()
  if(NOT ${EXIT_CODE_CYTHON} EQUAL 0)
    message(FATAL_ERROR
            "The \"cython\" Python3 package is not installed. Please install it using the following command: \"pip install Cython\"."
           )
  endif()

  # No ASan and other were already checked in CommonDefs
  if(S2OPC_NANO_PROFILE)
    message(SEND_ERROR "-- PyS2OPC requires S2OPC_NANO_PROFILE=0")
  endif()

  set(PYS2OPC_PXD "pys2opc.pxd")
  set(PYS2OPC_PYX "pys2opc.pyx")
  set(PYS2OPC_C "pys2opc.c")

  # ---------- Build PXD file ---------- #
  get_target_property(S2OPC_COMMON_INCLUDES s2opc_common INTERFACE_INCLUDE_DIRECTORIES)
  # Manage build interface issue when built at same time with S2OPC
  list(TRANSFORM S2OPC_COMMON_INCLUDES REPLACE "\\$<BUILD_INTERFACE:" "")
  list(TRANSFORM S2OPC_COMMON_INCLUDES REPLACE "\\$<INSTALL_INTERFACE:" "")
  list(TRANSFORM S2OPC_COMMON_INCLUDES REPLACE ">" "")
  # Add -I option flag to each elemen of the list
  list(TRANSFORM S2OPC_COMMON_INCLUDES PREPEND "-I")
  get_target_property(S2OPC_CLIENTSERVER_INCLUDES s2opc_clientserver INTERFACE_INCLUDE_DIRECTORIES)
  # Manage build interface issue when built at same with S2OPC
  list(TRANSFORM S2OPC_CLIENTSERVER_INCLUDES REPLACE "\\$<BUILD_INTERFACE:" "")
  list(TRANSFORM S2OPC_CLIENTSERVER_INCLUDES REPLACE "\\$<INSTALL_INTERFACE:" "")
  list(TRANSFORM S2OPC_CLIENTSERVER_INCLUDES REPLACE ">" "")
  # Add -I option flag to each elemen of the list
  list(TRANSFORM S2OPC_CLIENTSERVER_INCLUDES PREPEND "-I")

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_PXD}
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/s2opc_includes_pys2opc.h
                     COMMAND "${PYTHON_EXECUTABLE}" generate_pxd_autopxd.py
                      -DSKIP_S2OPC_DEFINITIONS=1
                      -DS2OPC_AUTOPXD=1
                      -I${CMAKE_CURRENT_SOURCE_DIR}
                      ${S2OPC_COMMON_INCLUDES}
                      ${S2OPC_CLIENTSERVER_INCLUDES}
                      s2opc_includes_pys2opc.h ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_PXD}
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/s2opc_non_auto.pxd ${CMAKE_CURRENT_BINARY_DIR}
# Uncomment the following line for development / debug purpose
#                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_PXD} ${CMAKE_CURRENT_SOURCE_DIR}
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     COMMENT "Generation of PyS2OPC PXD file: ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_PXD}"
                     )

  # Provoke rebuild of PXD in all cases since dependency on all S2OPC headers is not managed by command
  add_custom_target(pys2opc_pxd ALL
                    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_PXD}
                    )

  # ---------- Build C file ---------- #

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_C}
                     DEPENDS pys2opc_pxd ${PYS2OPC_PYX}
                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${PYS2OPC_PYX} ${CMAKE_CURRENT_BINARY_DIR}
                     COMMAND cython -3 ${PYS2OPC_PYX} -o ${PYS2OPC_C}
# Uncomment the following line for development / debug purpose (CAUTION: formating project when this file is present in sources might cause OOM)
#                     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_C} ${CMAKE_CURRENT_SOURCE_DIR}
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                     COMMENT "Generation of PyS2OPC C file from PXD/PYX files: ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_C}"
                    )


  # ---------- Build library ---------- #

  add_library(pys2opc SHARED ${CMAKE_CURRENT_BINARY_DIR}/${PYS2OPC_C})
  set_target_properties(pys2opc PROPERTIES PREFIX "")
  set(PYS2OPC_LINK_LIBRARIES s2opc_clientserver)
  if(WIN32)
    list(APPEND PYS2OPC_LINK_LIBRARIES ${PYTHON_LIBRARY})
    set_target_properties(
      pys2opc
      PROPERTIES
      SUFFIX ".pyd"
      )
  endif()
  target_link_libraries(pys2opc PUBLIC ${PYS2OPC_LINK_LIBRARIES})
  target_compile_definitions(pys2opc PRIVATE "SKIP_S2OPC_DEFINITIONS")
  target_include_directories(pys2opc PRIVATE ${PYTHON_INCLUDE_DIRS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

  set(PYS2OPC_BUILD_PATH "${CMAKE_BINARY_DIR}/pys2opc")
  file(MAKE_DIRECTORY ${PYS2OPC_BUILD_PATH}) # Note: create build directory to use it as working dir
  file(MAKE_DIRECTORY ${PYS2OPC_BUILD_PATH}/pys2opc)
  set(PYS2OPC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/__init__.py"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc.pyx"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/version.json"
                      )
  set(PYS2OPC_BUILDERS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
                       "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
                       "${CMAKE_CURRENT_SOURCE_DIR}/NOTICE"
                       "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"
                       )
  set(PYS2OPC_WHEEL_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PYS2OPC_WHEEL_NAME}")
  set(PYS2OPC_TEST_INSTALL_PATH "${CMAKE_BINARY_DIR}/pys2opc_install")

  add_custom_command(OUTPUT ${PYS2OPC_WHEEL_PATH}
                     DEPENDS pys2opc
                     # Note: sources copied to build dir to avoid relative path issues in pys2opc_build.py
                     COMMAND ${CMAKE_COMMAND} -E copy ${PYS2OPC_BUILDERS} ${PYS2OPC_BUILD_PATH}
                     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pys2opc> ${PYS2OPC_BUILD_PATH}/pys2opc
                     COMMAND ${CMAKE_COMMAND} -E copy ${PYS2OPC_SOURCES} ${PYS2OPC_BUILD_PATH}/pys2opc
                     COMMAND ${PYTHON_EXECUTABLE} -m build --outdir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} --wheel --no-isolation
                     WORKING_DIRECTORY ${PYS2OPC_BUILD_PATH}
                     COMMENT "Build PyS2OPC Wheel"
                     )

  # TODO: there should be a single target per dependency to avoid concurrent and parallel builds for them
  # This will be the case if the output files are re-used in another target.
  # (see add_custom_command and add_custom_target, or the target make-light-address-space)
  add_custom_target(pys2opc_wheel ALL
                    DEPENDS ${PYS2OPC_WHEEL_PATH}
                    )

  # Installation target
  # This is both required to test the installation of the wheel AND to be able to run the other tests AND to generate the doc...
  add_custom_command(OUTPUT ${PYS2OPC_TEST_INSTALL_PATH}/pys2opc
                     DEPENDS pys2opc_wheel
                     COMMAND ${PYTHON_EXECUTABLE} -m pip install -t ${PYS2OPC_TEST_INSTALL_PATH} --no-deps --no-index ${PYS2OPC_WHEEL_PATH}
                     COMMENT "Install (temporarly) PyS2OPC to ${PYS2OPC_TEST_INSTALL_PATH}"
                     )
  add_custom_target(pys2opc_install ALL
                    DEPENDS ${PYS2OPC_TEST_INSTALL_PATH}/pys2opc
                    )
  add_dependencies(pys2opc_install pys2opc_wheel)

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/apidoc/html
                     DEPENDS pys2opc_wheel
                     # Note: ignore the UserWarning lines generated by pdoc: "Couldn't read PEP-224 variable docstrings from <Class [...]: could not find class definition"
                     COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${PYS2OPC_TEST_INSTALL_PATH}" "PYTHONWARNINGS=\"ignore::UserWarning\""
                             ${PYTHON_EXECUTABLE} -m pdoc --config show_source_code=False --html pys2opc -fo ${CMAKE_BINARY_DIR}/apidoc/html
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                     )
  add_custom_target(pys2opc_doc
                    DEPENDS ${CMAKE_BINARY_DIR}/apidoc/html
                    )
  add_dependencies(pys2opc_doc pys2opc_install)

  if(TARGET doc)
    add_dependencies(doc pys2opc_doc)
  endif()

else()
  # We don't know if the module is built with pre-built S2OPC or is built in the context of S2OPC.
  # So we warn you that maybe you built S2OPC with some options that you forgot to repeat when trying to build PyS2OPC.
  message("-- PyS2OPC not compiled")
endif()
