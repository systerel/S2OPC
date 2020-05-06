# S2OPC Toolkit Test Server

This folder contains the code to run a simple OPC UA server on the NXP MIMXRT1064-EVK board.

## Description

This application is a simple OPC UA demo/test server.
It should run without issue on Zephyr.
It has been tested only on the MIMXRT1064 board.

The initial Proof-Of-Concept used patches in Zephyr kernel and MbedTLS crypto module.
This Zephyr kernel patches have been removed, and the MbedTLS ones have been moved to this application.

Security is currently not working, since we use the uptime instead of the system time.
Using system time requires POSIX API which make S2OPC compilation impossible.
This problem shall be solved as it will fix security and allow us to remove
the MbedTLS threading Alt and use POSIX threading instead.

## S2OPC as a Zephyr module patch

add
```
    - name: s2opc
      path: modules/lib/s2opc
      url: https://gitlab.com/systerel/S2OPC
      revision: <your_branch>
```
to Zephyr `west.yml` (in `projects`) and run `west update`.

## Compile

By default, the application is compiled for the MIMXRT1064-EVK board.

```
west build
```

## Flash

`west flash` does not work for the MIMXRT1064.
We need to copy the binary file into the board.
For example, on Ubuntu 18:
```
cp build/zephyr/zephyr.bin /media/<user>/RT1064-EVK
```

## Run setup and execution

* Open a serial monitor (`minicom` can be used on Ubuntu)
* Reset the board
* Use an OPC UA client to connect to the board OPC UA server.
