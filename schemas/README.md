Directory content:
- `Opc.Ua.NodeSet2.xml`: base UA node set (address space) defined by the OPC UA foundation for version 1.04 of specification, it is used as a base to define `samples/ClientServer/data/address_space/s2opc_base_nodeset_origin.xml` file
- `Opc.Ua.Types.bsd`: OPC UA Binary encoding for all DataTypes and Messages, it is used as a source file by `scripts/gen-sopc-types.py` to generate `src/Common/opcua_types/sopc_types[.h|.c]`
- `Opc.Ua.Types.xsd`: XML Schema for all DataTypes and Messages
- `OPCBinarySchema.xsd`: XML Schema for Opc.Ua.Types.bsd
- `sopc_clientserver_config.xsd`: XML schema for S2OPC server configuration file
- `sopc_clientserver_users_config.xsd`: XML schema for S2OPC server users configuration file
- `UANodeSet.xsd`: XML schema for address space definition used by S2OPC XML address space loaders