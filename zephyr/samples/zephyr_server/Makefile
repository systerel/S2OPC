export BOARD=stm32h735g_disco
export ZEPHYR_BASE=/workdir/zephyr
export CONF_FILE=prj.conf
export CMAKE_PREFIX_PATH=/opt/toolchains/zephyr-sdk-0.13.2/

export GEN_ADDRSPC=$(ZEPHYR_BASE)/../modules/lib/s2opc/scripts/generate-s2opc-address-space.py
all:regen rebuild
regen:
	@clear && printf '\e[3J' || true
	@ # Generate Static AddressSpace, based on XML file
	@$(GEN_ADDRSPC) xml/pubsub_server.xml src/test_address_space.c --const_addspace
	@cmake -B build
rebuild:
	@west build .
	@chmod -R auo+w build/*
	
clean:
	rm -rf build/

.PHONY:all regen rebuild clean