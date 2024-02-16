How to generate `s2opc.xml`, `s2opc_nano.xml`, `s2opc_sks.xml` and `s2opc_push.xml` : the 2 first files are generated from `s2opc_base_nodeset_origin.xml` and `s2opc_demo_data_origin.xml` using `nodeset-address-space-utils.py` tool

s2opc.xml: merge OPCUA base nodeset and demo application nodeset and generate reciprocal references
`<S2OPC_root>/scripts/nodeset_address_space_utils.py --output s2opc.xml s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml s2opc_demo_data_perfs.xml`

`s2opc_nano.xml`: merge OPCUA base nodeset and demo application nodeset, generate reciprocal references and remove nodes that shall not be present for Nano profile
`<S2OPC_root>/scripts/nodeset_address_space_utils.py --output s2opc_nano.xml --remove-max-monitored-items --remove-methods --remove-max-node-management s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml  s2opc_demo_data_perfs.xml`

`s2opc_no_base_info.xml`: merge OPCUA base nodeset and demo application nodeset, generate reciprocal references and remove nodes for an address space with limited NS0 base info data:
`<S2OPC_root>/scripts/nodeset_address_space_utils.py --output s2opc_no_base_info.xml --remove-node-ids-greater-than 3000 --remove-methods s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml`

`<S2OPC_root>/scripts/nodeset_address_space_utils.py --output s2opc_nano.xml --remove-max-monitored-items --remove-methods s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml`

`s2opc_sks.xml`: merge OPCUA base nodeset, sks base nodeset (extract) and demo application nodeset, generate reciprocal references
`<S2OPC_root>/scripts/nodeset_address_space_utils.py --output s2opc_sks.xml s2opc_base_nodeset_origin.xml s2opc_base_sks_origin.xml s2opc_demo_data_origin.xml`

`s2opc_push.xml`: merge OPCUA base nodeset, push base nodeset (extract) and demo application nodeset, generate reciprocal references
`<S2OPC_root>/scripts/nodeset-address-space-utils.py --output s2opc_push.xml s2opc_base_nodeset_origin.xml s2opc_base_push_server_origin.xml s2opc_demo_data_origin.xml`

See `./generate_demo_nodesets.sh` for exact command lines for each XML NodeSet generated in this directory.
