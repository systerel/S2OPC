#!/usr/bin/env python3

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# Main script for HIL testing
# Takes all information needed from build_config.json, test_to_launch.json; hardware_capa.json
# Launches successively ./compile.sh, ./flash_app.sh and then the tests with executor.py
# Returns 0 and OK in case of success
# Returns 1 and FAIL in case of failures

import os
import sys
import json
import subprocess
from pathlib import Path

def usage():
    print("Builds and flash applications for a given OS using")
    print("Usage: script.py [--os <OS>]")
    print(" --os <OS> : which operating system is being targeted, default zephyr")
    print(" -h : Print this help and return")
    sys.exit(0)

def fail(message):
    print(f"[EE] {message}", file=sys.stderr)
    sys.exit(1)

# Handle CLI arguments
args = sys.argv[1:]
if "-h" in args:
    usage()

target_os = "zephyr"
if "--os" in args:
    os_index = args.index("--os")
    if os_index + 1 < len(args):
        target_os = args[os_index + 1]
    else:
        fail("Missing argument for --os")
else:
    print(f"Using default OS {target_os}")

# Setup directories
hil_dir = Path(__file__).resolve().parent
host_dir = hil_dir.parent.parent
emb_dir = host_dir / "samples" / "embedded"

build_dir = host_dir / f"build_{target_os}"
if build_dir.exists():
    for f in build_dir.glob("*"):
        if f.is_file():
            f.unlink()
        elif f.is_dir():
            subprocess.run(["rm", "-rf", str(f)], check=False)
else:
    build_dir.mkdir(parents=True, exist_ok=True)

# Load config files
build_cfg_list = hil_dir / "config" / "build_config.json"
test_name_cfg = hil_dir / "config" / "test_to_launch.json"
hardware_capacity = hil_dir / "config" / "hardware_capa.json"

with open(test_name_cfg) as f:
    test_launch_config = json.load(f)

test_name_list = test_launch_config.get("launch_tests", [])
if not test_name_list:
    fail(f"Missing 'test_name' in test_to_launch.json ({test_name_list})")

with open(build_cfg_list) as f:
    build_config = json.load(f)

for test in test_name_list:
    builds = build_config["tests"].get(test, {}).get("builds", [])
    for index, build in enumerate(builds):
        serial = build.get("BOARD_SN")
        if not serial:
            fail(f"Missing 'BOARD_SN' field in 'builds' ({build})")
        build_name = build.get("build_name")
        if not build_name:
            fail(f"Missing 'BUILD_NAME' field in 'builds' ({build})")
        build_info = build_config["build"].get(build_name, {})
        os_val = build_info.get("OS")
        if not os_val:
            fail(f"Missing 'OS' field in 'builds' ({build})")

        if os_val != target_os:
            continue

        platform_dir = emb_dir / "platform_dep" / os_val
        if not platform_dir.is_dir():
            fail(f"OS '{os_val}' is not supported on HIL tests")

        app = build_info.get("app")
        if not app or not (emb_dir / app).is_dir():
            fail(f"APP '{app}' is not supported or missing on HIL tests")

        board = build_info.get("board")
        if not board:
            fail(f"Missing 'BOARD' field in 'builds' ({build})")
        board_name = board.replace("/", "_")

        extension = build_info.get("flash_type")
        if not extension:
            fail(f"Missing 'EXTENSION' field in 'builds' ({build})")

        ip_address = build_info.get("IP_ADDRESS")
        if not ip_address:
            fail(f"Missing 'IP_ADDRESS' field in 'builds' ({build})")

        log_file = build_dir / f"{app}_{board_name}.log"
        out_file = build_dir / f"{app}_{board_name}.{extension}"

        if not out_file.exists():
            print(f"Building {out_file.name} for board {board}")
            build_script_dir = host_dir / "samples" / "embedded" / "platform_dep" / os_val / "ci"
            build_script = build_script_dir / f"build-{os_val}-samples-docker.sh"
            if not build_script_dir.is_dir():
                fail(f"Missing folder '{build_script_dir}'")
            if not os.access(build_script, os.X_OK):
                fail(f"Missing or invalid build script '{build_script}'")

            try:
                subprocess.run(
                    [str(build_script), board, app,
                     "--ip", ip_address,
                     "--log", str(log_file),
                     "--bin", str(out_file)],
                    check=True,
                    cwd=str(build_script_dir)
                )
            except subprocess.CalledProcessError:
                fail(f"Failed running build script '{build_script}'")

            if not out_file.exists():
                fail(f"Missing output file {out_file} (see {log_file})")
        else:
            print(f"Not rebuilding {out_file.name}")

# Final check
if not any(build_dir.iterdir()):
    print(f"Warning: no builds were run for {target_os}")
