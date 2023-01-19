# Description
The samples in this folder are specific for embedded design. In particular:
- No filesystem is used (implying that the configuration is programmatic rather than XML-based
- No ARGC/ARGV feature is supposed to be available. Demos may however be interactive using Client-Line interface.

All the target-specific features (such as hardware and network setup) are abstracted in the folder
platform_dep.

# Organization
The file organisation is as follow:
platform_dep/CMake      : default CMake configuration
platform_dep/include    : Header of target-dependant features
platform_dep/tls_config : Specific EmbedTLS configuration for embedded (common code to all platforms)
platform_dep/<os>       : Specific implementation for given <platform>. It must at least implement
                          the features defined in platform_dep/include. It also can provide some OS-specific
                          configuration or build rules

# Currently supported targets
## Linux/Windows
Uses the default CMake rules. These samples are automitically build when calling the root `build.sh` s2opc script.

## Zephyr
Uses a Zephyr-dedicated CMake configuration.
To build a sample, enter the sample folder using a ZEPHYR-configured terminal (`ZEPHYR_BASE` must be defined)
and type `west build`.

## FreeRTOS
To be completed

