# S2OPC Toolkit Test Server

This folder contains the code to run a simple OPC UA server on Zephyr (by
default, uses NXP MIMXRT1064-EVK board).

It is known to be working with Zephyr tag `zephyr-v2.2.0`.

## Description

This application is a simple OPC UA demo/test server.
It should run without issue on Zephyr.

It has been tested on the MIMXRT1064 board.
In order for S2OPC to be integrated as a Zephyr module, it should also run on
`qemu_x86`.

The initial Proof-Of-Concept used patches in Zephyr kernel and MbedTLS crypto
module.
This Zephyr kernel patches have been removed, and the MbedTLS ones have been
moved to this application.

Security is currently working, but we use the build time while waiting for zephyr
issue #24730 to be solved.  It would allow us to use the system time.
(Using system time requires POSIX API which make S2OPC compilation impossible.)
This problem shall be solved as it will fix security and allow us to remove
the MbedTLS threading Alt and use POSIX threading instead.

Remaining warnings should also be fixed.

The `zephyr/CMakeLists.txt` should also be removed/modified to use the main S2OPC
`CMakeLists.txt`. `zephyr/Kconfig` should also include additional configuration
options for S2OPC.

## S2OPC as a Zephyr module patch

add
```
    - name: s2opc
      path: modules/lib/s2opc
      url: https://gitlab.com/systerel/S2OPC
      revision: zephyr
```
to Zephyr `west.yml` (in `projects`) and run `west update`.

## Compile

By default, the application is compiled for the MIMXRT1064-EVK board.

```
west build
```

To specify your board, use the `-b` option or the `BOARD` environment varible.

## Flash

`west flash`

However, it does not work for the MIMXRT1064.
We need to copy the binary file into the board.
For example, on Ubuntu 18:
```
cp build/zephyr/zephyr.bin /media/<user>/RT1064-EVK
```

## Run setup and execution

* Open a serial monitor (`minicom` can be used on Ubuntu)
* Reset the board
* Use an OPC UA client to connect to the board OPC UA server.
