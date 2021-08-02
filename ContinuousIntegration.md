# Introduction

The purpose of this file is to explain what are the steps needed to launch integration tests for S2OPC running on emulated boards with renode. The goal is not to document every technology used but to show the major step to understand what has been done and easily reproduce and adapt this as needed.

The continuous integration pipeline kicks on anytime a push occur. The steps that happen for testing with renode are `build-publisher` and `build-subscriber` in the stage `build` to build an application that will be installed on an emulated board with renode and `test-renode` in the stage `test` to launch the tests on this application.

All the jobs use the same docker image (`registry.gitlab.com/systerel/s2opc/test_renode:latest`) which contains all the necessary to build (zephyr sources) and test (renode) this application. Gitlab will put the sources in the container at the location `/builds/systerel/S2OPC`. Since zephyr needs to have access to every source file needed for the application inside the folder `zephyrproject`, a link is made from `zephyrproject/modules/lib/s2opc` to `/builds/systerel/S2OPC` to access the source of `S2OPC` inside the folder `zephyrproject`. The link is always present in the image but becomes valid when gitlab pushes the source inside the container.

# Build Stage

The application built is `samples/PubSub_ClientServer/zephyr/pubsub_test_server`. There are two build jobs: one that builds the application on publisher mode (only), and the other one on subscriber mode (only). The build is made by two scripts to perform the specific required actions : `samples/PubSub_ClientServer/zephyr/pubsub_test_server/renode/scripts/pub.build.sh` to build the publisher and `sub.build.sh` to build the subscriber in the same folder.

The build actions are as follow :
1. Remove previous build output (useful for testing on local machine but not needed inside docker)
2. Copy the files `pubproj.conf` (for configuring the publisher) or `subproj.conf` (for configuring the subscriber) as `prj.conf` (needed by zephyr)
3. Call the script `lib/s2opc/scripts/generate-s2opc_pubsub-static-config.py` to generate the file `pubsub_config_static.c` based on the content of `pubconfig.xml` for the publisher and `subconfig.xml` for the subscriber.
4. Call `west` to build the app for the sam_e70 board. The outputs are placed inside the folder `/builds/systerel/S2OPC` because gitlab only accept artefacts inside this folder.

The build outputs are then configured as artefacts to be used inside the `test-renode` job

# Test Stage

The first script called is `prepare.sh`. It calls renode on the command line to create a tap interface on the container and configures it. We can't configure the tap interface inside renode, so we used renode to create it in the command line and close renode. The last parameter of the `CreateTap` command (`true`) make the tap interface last on the machine even if renode is closed.

Then a value is assigned to the `PYTHONPATH` environment variable, otherwise, the python scripts used by the python module we need won't be found.

Then `renode-test` is called to interpret the robot test cases written inside the file `test.robot`. This command executes robot, launches a renode instance, and add renode capabilities to robot. The `--result-dir` parameter configures robot to save its outputs inside `/builds/systerel/S2OPC` so that we can use the output files in the artefacts.

## Creating Test In Robot

Robot is a keyword-based test and automation framework. It allows the user to write their own keywords. To separate keywords, robot use double spaces or tab, so a keyword can contain spaces : `A Long Robot Keyword` is valid and parsed as one keyword. `Robot Keyword <space> <space> Argument One` is one keyword : `Robot Keyword` taking one parameter : `Argument One`.

The user can write new tests in (among other languages) a python file. The tests must be written as functions, in a class with the same name of the file. Thus, for a file `S2OPCTest.py`, the tests are in `class S2OPCTest`. Robot will translate the functions of this class as valid keywords using the following rules :
* all letters of the python function names must be lower case
* all words of the python function name must be separated by `'_'` (underscore)

When parsing a robot script, robot associates each python function following those rules to a keyword by
* replacing all `'_'` by a space
* making all words of the function starting by the same uppercase letter

For example: `wait_publisher` in a python file becomes `Wait Publisher` in robot file. `test_static_configuration` in a python file becomes becomes  `Test Static Configuration` in robot.

To import the python file with the tests, we use the `Library` keywords. `Library` takes the path to the python file **relative to** the robot file being parsed : `Library         S2OPCTest.py` import the file "`S2OPCTest.py`" located at the same level of the robot script.

(only the functions intended to be used in robot must follow those rules)

## The script test.Robot

This script launches all the tests. The first 3 lines are required. `Library          S2OPCTest.py` imports the python files where we call S2OPC tests.

Then we create 2 tests cases: one to verify that we can connect to the publisher and the other to verify that the communication between publisher and subscriber works well.

On both tests :
* `Execute Command` is a keyword added by renode to execute a renode command. We execute the command `path add @<path>` to add a path for renode to search the executables
* `Execute Script` is a keyword added by renode that permits to load a configuration script to create VMs in renode; here we load `ci.repl`
* `Start Emulation` is a keyword added by renode to launch the emulation

After those steps we call the keywords we defined inside `S2OPCTest.py` that performs the tests.

## Renode Script
In the file `ci.repl` we write the commands we want to pass to renode to configure the emulation. This script creates a switch, creates two VMs (pub and sub) and loads the corresponding executable into it (provided by previous jobs).  It connects each VMs to the switch, and connect the tap interface created and configured previously in `prepare.sh` to the switch. This way, the host can communicate with the VMs emulated by renode.

---

When `renode-test` finished its execution, the last step of the ci pipeline is to move any additional files inside `/builds/systerel/S2OPC` to make them available as artefacts.

When the execution of the robot tests are done, the output files permit to view result as HTML web pages. Their are available in the "artifacts section of gitlab"