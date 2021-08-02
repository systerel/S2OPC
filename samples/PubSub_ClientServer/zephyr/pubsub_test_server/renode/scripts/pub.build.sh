#Preparing for building
rm -Rf /builds/systerel/S2OPC/pub
#cp ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/data/config/pubconfig.xml ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/data/config/config_pubsub_server.xml
cp ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/pubprj.conf ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/prj.conf

#Generating static configuration file
python3 modules/lib/s2opc/scripts/generate-s2opc_pubsub-static-config.py modules/lib/s2opc/samples/PubSub_ClientServer/data/config/config_pubsub_server.xml modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/src/pubsub_config_static.c

#building
west build -p -b sam_e70_xplained --build-dir /builds/systerel/S2OPC/pub ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server -- -DPUBSUB_STATIC_CONFIG=ON
