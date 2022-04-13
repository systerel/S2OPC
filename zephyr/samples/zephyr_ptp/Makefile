export BOARD=nucleo_h745zi_q_m7
export ZEPHYR_BASE=/workdir/zephyr
export CONF_FILE=prj.conf
export CMAKE_PREFIX_PATH=/opt/toolchains/zephyr-sdk-0.13.2/

all:
	@clear && printf '\e[3J' || true
	@west build .
	@chmod -R auo+w build/*
	@cp build/zephyr/zephyr.bin /workdir/zephyr_$(BOARD)_ptp.bin
	
clean:
	rm -rf build/

.PHONY:all clean