export BOARD=stm32h735g_disco
export ZEPHYR_BASE=/workdir/zephyr
export CONF_FILE=prj.conf
export CMAKE_PREFIX_PATH=/opt/toolchains/zephyr-sdk-0.13.2/

all:
	@clear && printf '\e[3J' || true
	@west build .
rebuild:
	@echo "Rebuild $$(pwd)"
	@clear && printf '\e[3J' || true
	@cmake -B build
	
clean:
	@echo "Cleaning $$(pwd)"
	@rm -rf build/ || true

.PHONY:all