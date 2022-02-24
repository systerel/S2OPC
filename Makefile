DOCKER_IMAGE=sha256:1d0eb4c4f992
all:
	./.run-in-docker.sh $(DOCKER_IMAGE) DOCKER_IMAGE=$(DOCKER_IMAGE) CMAKE_TOOLCHAIN_FILE=/toolchain-rpi.cmake BUILD_DIR=build_rpi WITH_NANO_EXTENDED=1 ./build.sh