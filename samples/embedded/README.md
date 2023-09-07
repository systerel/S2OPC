# Description
The samples in this folder are specific for embedded design. In particular:
- No filesystem is used (implying that the configuration is programmatic rather than XML-based)
- No ARGC/ARGV feature is supposed to be available. Demos may however be interactive using Command Line Interface.
- They are designed so that they do not rely too much on hardware-specific features, and thus are portable on several O.S. All the target-specific features (such as hardware and network setup) are abstracted in the folder `platform_dep`.

# Organization
The file organisation is as follow:
- `platform_dep/CMake`      : default CMake configuration
- `platform_dep/include`    : Header of target-dependant features
- `platform_dep/mbedtls_config` : Specific MbedTLS configuration for embedded (common code to all platforms)
- `platform_dep/<os>`       : Specific implementation for given <platform>. It must at least implement
-                           the features defined in `platform_dep/include`. It also can provide some OS-specific configuration or build rules.

# Currently supported targets
## Linux/Windows
Uses the default CMake rules. These samples are automatically built when calling the root `build.sh` s2opc script.
Build outputs are located in `build/bin` folder.

## Zephyr
Uses a Zephyr-dedicated CMake configuration. (See `platform_dep/zephyr/CMakeLists.txt`).
In cas of Zephyr build, the `platform_dep/zephyr/CMakeLists.txt` rules are used (included from `platform_dep/CMake/CMakeLists.txt`). The objective here is to use the same build rules for all embedded demos and avoid duplication of complex CMake rules for each possible target of each sample. Refer to [wiki](https://gitlab.com/systerel/S2OPC/-/wikis/compilation/Zephyr-compilation) for more explanations on how to create a Zephyr project with S2OPC.

To build a sample, enter the sample folder using a ZEPHYR-configured terminal (`ZEPHYR_BASE` and `BOARD` must be defined and exported) and type `west build`.
Build outputs are located in `build/bin` folder.

Note that for Zephyr samples, the CLI is included within Zephyr SHELL under the SHELL subcommand `sopc`. Type `sopc help` to get the command list.

## FreeRTOS
Based on STMCube Ide. It only supports STM32H723ZG currently and acts as a proof of concept.
- To build the sample natively, follow instructions on [WIKI](https://gitlab.com/systerel/S2OPC/-/wikis/compilation/FreeRTOS-compilation)
- To build the sample with the docker, simply run the script `platform_dep/freertos/ci/build-freertos-samples.sh`
In all cases, refer to the wiki for limitations and other target ports.

# To add new supported targets
- Create a folder `platform_dep/<os>/`
- Implement the services described in `platform_dep/include/` in  `platform_dep/<os>/src/`
- If the CMake configuration is not compatible or if the build is specific, add the specific rules in `platform_dep/<os>/`
