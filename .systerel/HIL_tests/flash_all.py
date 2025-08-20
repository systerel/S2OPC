#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

# Flash script for HIL testing
# Takes all information needed from build_config.json, test_to_launch.json; hardware_capa.json
# Flash the application onto the appropriate board
# Then run the asociated test and repeat

import shutil, os
import sys
import json
import subprocess
from pathlib import Path
import executor

def fail(message):
    print(f"[EE] {message}", file=sys.stderr)
    sys.exit(1)

# Setup directories
hil_dir = Path(__file__).resolve().parent
host_dir = hil_dir.parent.parent
emb_dir = host_dir / "samples" / "embedded"

log_dir = host_dir / 'log'
if os.path.isdir(log_dir):
    shutil.rmtree(log_dir)
os.makedirs(log_dir)

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
        platform_dir = emb_dir / "platform_dep" / os_val
        if not platform_dir.is_dir():
            fail(f"OS '{os_val}' is not supported on HIL tests")
        build_dir = host_dir / f"build_{os_val}"
        if not build_dir.is_dir():
            fail(f"{build_dir} was not found")

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

        crypto = build_info.get("crypto")
        if not crypto:
            fail(f"Missing 'IP_ADDRESS' field in 'builds' ({build})")

        log_file = log_dir / f"flash_{app}_{board_name}_{crypto}.log"
        os.makedirs(os.path.dirname(log_file), exist_ok=True)
        bin_file = f"{app}_{board_name}_{crypto}.{extension}"

        if not (build_dir / bin_file).exists():
            fail(f"Missing output file {(build_dir / bin_file)}")
        else:
            print(f"Flash {app}/{os_val} on board {board} SN={serial}")
            flash_app = hil_dir / "flash_app.sh"
            with open(log_file, "w") as log:
                try:
                    subprocess.run(
                        [str(flash_app), serial, bin_file, os_val],
                        check=True,
                        cwd=str(hil_dir),
                        stdout=log,
                        stderr=subprocess.STDOUT
                    )
                except subprocess.CalledProcessError:
                    with open(log_file, "r") as log_read:
                        print(log_read.read())
                    fail(f"Failed running flash script '{flash_app}'")

    print(f"Running test {test}")
    log_file = log_dir / f"execute_{app}_{board_name}_{crypto}.log"
    try:
        executor.run_test(test, build_cfg_list, hardware_capacity, log_file)
    except subprocess.CalledProcessError:
        fail(f"Failed running test '{test}'")

print(f"All test OK")
