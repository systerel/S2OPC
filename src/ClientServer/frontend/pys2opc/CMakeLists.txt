cmake_minimum_required (VERSION 3.5)
project (S2OPC_PyS2OPC)

if(NOT S2OPC_COMMON_DEFS_SET)
  include(../../../../CommonDefs.cmake)
endif()

##                                                              ##
# PyS2OPC library: Python 3 binding for S2OPC client and servers #
##                                                              ##

# pip lacks the ability to produce the name of the output library before it is done,
#  so it is hardcoded.
set(PYS2OPC_WHEEL_NAME "pys2opc-0.1.1-cp36-cp36m-linux_x86_64.whl" CACHE STRING
    "The guessed name of the produced Python 3 Wheel. It must be set to the correct name of the wheel that will be produced in build/lib for tests to run.")

### Dependencies ###

# EXPAT dependency managed in CommonDefs

# use find_package onto s2opc in case we build s2opc and PyS2OPC separately
find_package(s2opc)

# Python3 is mandatory for compilation
find_package(PythonInterp 3 REQUIRED)

# The library is built by python. It may use the shared library but it must not be built with ASan.
# The problem with ASan is to embed a dynamic library compiled with ASan into Python, which is not compiled with ASan.
# The compilation is not done with clang-analyzer either, as this would analyze CFFI-generated code.
if(WITH_PYS2OPC)
  # TODO:
  # - find how to predict the wheel name and remove the PYS2OPC_WHEEL_NAME option
  # - do a pys2opc_build.py.in to configure the lib through CMake, remove manual copy of pys2opc
  # - check library presence from s2opc's exports and not from flags defined in CommonDefs
  # - make this file really independent from the rest of the project, but this requires more properties to be exported from clientwrapper.

  # Check requirements
  # No ASan and other were already checked in CommonDefs
  if(NOT WITH_NANO_EXTENDED)
    message(SEND_ERROR "-- PyS2OPC requires WITH_NANO_EXTENDED")
  endif()
  if(NOT TARGET s2opc_clientwrapper OR NOT TARGET s2opc_clientserver-xml-loaders-expat)
    message(SEND_ERROR "-- PyS2OPC requires the S2OPC client wrapper and S2OPC xml loaders with Expat")
  endif()
  # No Warning on PUBSUB_ONLY, as this file is not loaded in this case...

  set(PYS2OPC_BUILD_PATH "${CMAKE_BINARY_DIR}/pys2opc")
  file(MAKE_DIRECTORY ${PYS2OPC_BUILD_PATH}) # Note: create build directory to use it as working dir
  file(MAKE_DIRECTORY ${PYS2OPC_BUILD_PATH}/pys2opc)
  set(PYS2OPC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/__init__.py"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/connection.py"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/request.py"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/responses.py"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/s2opc.py"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/server_callbacks.py"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/types.py"
                      "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc/version.json"
                      )
  set(PYS2OPC_BUILDERS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
                       "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
                       "${CMAKE_CURRENT_SOURCE_DIR}/NOTICE"
                       "${CMAKE_CURRENT_SOURCE_DIR}/pys2opc_build.py"
                       "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"
                       )
  set(PYS2OPC_FILES ${PYS2OPC_SOURCES} ${PYS2OPC_BUILDERS})
  #set(PYS2OPC_PLACEHOLDER ${CMAKE_BINARY_DIR}/pys2opc_touch)
  set(PYS2OPC_INCLUDE "${PYS2OPC_BUILD_PATH}/s2opc_expanded.h")
  set(PYS2OPC_WHEEL_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PYS2OPC_WHEEL_NAME}")
  set(PYS2OPC_TEST_INSTALL_PATH "${CMAKE_BINARY_DIR}/pys2opc_install")

  # Produce the expanded header
  s2opc_expand_header(${CMAKE_CURRENT_SOURCE_DIR}/s2opc_headers.h
                      "s2opc_clientwrapper;s2opc_clientserver-xml-loaders-expat"
                      ${PYS2OPC_INCLUDE}
                      )

  add_custom_command(OUTPUT ${PYS2OPC_WHEEL_PATH}
                     DEPENDS ${PYS2OPC_INCLUDE}
                             ${PYS2OPC_FILES}
                     # Note: sources copied to build dir to avoid relative path issues in pys2opc_build.py
                     COMMAND ${CMAKE_COMMAND} -E copy ${PYS2OPC_BUILDERS} ${PYS2OPC_BUILD_PATH}
                     COMMAND ${CMAKE_COMMAND} -E copy ${PYS2OPC_SOURCES} ${PYS2OPC_BUILD_PATH}/pys2opc
                     COMMAND ${PYTHON_EXECUTABLE} ${PYS2OPC_BUILD_PATH}/setup.py -q bdist_wheel
                             -d ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                     #COMMAND ${CMAKE_COMMAND} -E touch ${PYS2OPC_PLACEHOLDER}
                     WORKING_DIRECTORY ${PYS2OPC_BUILD_PATH}
                     COMMENT "Build PyS2OPC Wheel"
                     )

  # TODO: there should be a single target per dependency to avoid concurrent and parallel builds for them
  # This will be the case if the output files are re-used in another target.
  # (see add_custom_command and add_custom_target, or the target make-light-address-space)
  add_custom_target(pys2opc ALL
                    DEPENDS ${PYS2OPC_WHEEL_PATH}
                    )
  add_dependencies(pys2opc s2opc_clientwrapper)
  add_dependencies(pys2opc s2opc_clientserver-xml-loaders-expat)

  # Installation target
  # This is both required to test the installation of the wheel AND to be able to run the other tests AND to generate the doc...
  add_custom_command(OUTPUT ${PYS2OPC_TEST_INSTALL_PATH}/pys2opc
                     DEPENDS ${PYS2OPC_WHEEL_PATH}
                     COMMAND ${PYTHON_EXECUTABLE} -m pip install -t ${PYS2OPC_TEST_INSTALL_PATH} --no-deps --no-index ${PYS2OPC_WHEEL_PATH}
                     COMMENT "Install (temporarly) PyS2OPC to ${PYS2OPC_TEST_INSTALL_PATH}"
                     )
  add_custom_target(pys2opc_install ALL
                    DEPENDS ${PYS2OPC_TEST_INSTALL_PATH}/pys2opc
                    )
  add_dependencies(pys2opc_install pys2opc)

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/apidoc/html
                     DEPENDS ${PYS2OPC_WHEEL_PATH}
                     COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${PYS2OPC_TEST_INSTALL_PATH}"
                             ${PYTHON_EXECUTABLE} -m pdoc --html pys2opc -fo ${CMAKE_BINARY_DIR}/apidoc/html
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
