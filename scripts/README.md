Directory content:
  - `gen_build_info_file.sh`: automatically run during Linux compilation to generate `csrc/configuration/sopc_toolkit_build_info.h` containing build information
  - `generate-embedded-base-type-info.py`: manually run to generate `csrc/opcua_types/sopc_embedded_nodeset2.h` from OPC UA foundation `Opc.Ua.NodeSet2.xml` file
  - `generate-s2opc-address-space.py`: manually run on an address space XML file compliant with `UANodeSet.xsd` schema to generate a C source file defining an address space that can be loaded using `csrc/loaders/address_space_loaders/embedded loader`
  - `gen-reciprocal-refs-address-space.xslt`: manually run on an address space XML file compliant with `UANodeSet.xsd` schema to generate all reciprocal references in the address space (all reciprocal references shall be defined to be used as address space configuration)
  - `gen-sopc-types.py`: manually run to generate `csrc/opcua_types/sopc_types[.h|.c]` from OPC UA foundation `Opc.Ua.Types.bsd` file
  - `remove-methods-address-space.xslt`: manually run on an address space XML file compliant with `UANodeSet.xsd` schema to remove all method instance nodes (MethodDeclarationId attribute shall be declared) and their arguments
