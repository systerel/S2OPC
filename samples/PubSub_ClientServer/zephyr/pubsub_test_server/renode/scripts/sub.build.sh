#Getting the last version of s2opc
west update

#Preparing for building
rm -Rf sub
cp ~/subconfig.xml ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/data/config/config_pubsub_server.xml
cp ~/subprj.conf ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/prj.conf

#Generating static configuration file
python3 modules/lib/s2opc/scripts/generate-s2opc_pubsub-static-config.py modules/lib/s2opc/samples/PubSub_ClientServer/data/config/config_pubsub_server.xml modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/src/pubsub_config_static.c

#building
west build -b sam_e70_xplained --build-dir /builds/systerel/S2OPC/sub ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server -- -DPUBSUB_STATIC_CONFIG=ON

#moving the artifacts were Gitlab can download it
mv pub/zephyr/zephyr.elf /builds/systerel/S2OPC/sub.zephyr.elf