How to generate `s2opc.xml` and `s2opc_nano.xml`: the 2 files are generated from `s2opc_base_nodeset_origin.xml` and `s2opc_demo_data_origin.xml` using `nodeset-address-space-utils.py` tool

s2opc.xml: merge OPCUA base nodeset and demo application nodeset and generate reciprocal references
`<S2OPC_root>/scripts/nodeset-address-space-utils.py --output s2opc.xml s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml`

`s2opc_nano.xml`: merge OPCUA base nodeset and demo application nodeset, generate reciprocal references and remove nodes that shall not be present for Nano profile
`<S2OPC_root>/scripts/nodeset-address-space-utils.py --output s2opc_nano.xml --remove-max-monitored-items --remove-methods s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml`
