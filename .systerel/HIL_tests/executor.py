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

import sys, os, json, re, logging
from pathlib import Path
from time import sleep
from datetime import datetime
import subprocess

DEFAULT_SUBPROCESS_TIMEOUT=5

# Uncomment print to activate debug messages
def _DEBUG(x):
    #print(f"{datetime.now() } [SCENARIO] " + x, flush=True)
    pass

def safe_json_load(filepath):
    try:
        with open(filepath) as file:
            return json.load(file)
    except Exception:
        raise Exception(f"Invalid JSON file {filepath}")


def get_json_arg(json_data, keys):
    current_level = json_data
    for key in keys:
        if key not in current_level:
            raise Exception(f"Missing '{key}' field in JSON data")
        current_level = current_level[key]
    return current_level


def run_test(test_key, build_config_path, hardware_capa_path, log_file):
    logger = logging.getLogger(test_key)
    if not logger.hasHandlers():
        logging.basicConfig(filename=Path(log_file),
                            filemode='w',
                            format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
                            datefmt='%H:%M:%S',
                            level=logging.INFO)
    try:
        build_config_json = safe_json_load(Path(build_config_path))
        hardware_capa_json = safe_json_load(Path(hardware_capa_path))

        logger.info(f"Running test : {test_key}")
        executor = Executor(build_config_json, test_key, logger, hardware_capa_json)

        scenario_path = Path(__file__).parent / f"scenario/{executor.test_script}"
        scenario_data = safe_json_load(scenario_path)
        script_actions = parse_scenario(scenario_data)

        executor.execute(script_actions)
    except Exception as E:
        logger.error(f"{E}")
        _DEBUG(f"Execution failure : {E}")
        sys.exit(1)

class ScriptAction:
    def __call__(self, executor):
        raise NotImplementedError("Subclasses must implement this method")

class ScriptActionSleep(ScriptAction):
    '''
        This derivation of ScriptAction implements the Sleep Action
    '''

    def __init__(self, target_name, t):
        super().__init__()
        self.target_name=target_name
        self.t=t

    def __call__(self, executor):
        proc = executor.procs[self.target_name]
        proc.sleep(self.t)

class ScriptActionWrite(ScriptAction):
    '''
        This derivation of ScriptAction implements the Write Action
    '''
    def __init__(self, target_name:str, text : str):
        super().__init__()
        self.text=text
        self.target_name=target_name

    def __call__(self, executor):
        proc = executor.procs[self.target_name]
        proc.write(self.text)

class ScriptActionRead(ScriptAction):
    '''
        This derivation of ScriptAction implements the Read Action
    '''
    def __init__(self, target_name:str, textRef : str, timeout:float):
        super().__init__()
        self.textRef=textRef
        self.target_name=target_name
        self.timeout=timeout

    def __call__(self, executor):
        proc = executor.procs[self.target_name]
        proc.read(self.textRef, self.timeout)

class RemoteProcess:
    def __init__(self, build_info, test_info, logger, hardware_capa_json):
        '''
        @param build_info A specific dict build information which must contain:
                "OS": The operating system
                "app": The application name (must be one of samples/embedded application)
                "board": board name 'must match one of 'capabilities' field in "harware_capa.json"
                "crypto": the crypto layer ot use ("mbedtls" or "none" TODO, y en a-ti-l d'autre?)
                "dynamic_ip": "true" or "false". Indicates if the IP can be dynamically set
                "flash_type": "bin" or "elf"
        @param test_info A specific dict test information which must contain:
                "name": A more explicit name for the board that will be used
                "build_name": The name of the build that will be used, it directly refers to param build_info
                "BOARD_SN": The serial number of the board that will be used
        '''
        self.build_info = build_info
        self.hardware_capa_json = hardware_capa_json
        self.logger = logger
        self.SN = get_json_arg(test_info,['BOARD_SN'])
        self.port=None
        board_name = get_json_arg(build_info,["board"])
        _os = get_json_arg(build_info,["OS"])
        self.comm_protocol, self.comm_options \
            = get_json_arg(hardware_capa_json,["capabilities",board_name,"comm_config"])
        self.format=get_json_arg(hardware_capa_json,["os_specificities",_os,"target_write_prefix"])

        try:
            script_path = Path(__file__).parent / "comm" / self.comm_protocol
            script_path = str(script_path.resolve())
            if not os.path.isfile(script_path):
                logger.error(f"File not found : {script_path}")
                raise
        except Exception as E:
            logger.error(E)
            raise Exception(f"Unknown communication protocol {self.comm_protocol} for board {board_name}")

        result = subprocess.run([script_path, 'get_dev', self.SN], capture_output=True, text=True)
        port = result.stdout.strip()
        _DEBUG(port)
        if port:
            logger.info(f"Detected port {port}")
            self.port = port
            self.script_path = script_path
            subprocess.run([script_path, 'init_port', port, self.comm_options], check=True)
            _DEBUG(f"Port : {port} configured")
        else:
            msg=f"No port detected for Board SN={self.SN}"
            _DEBUG(msg)
            logger.error(msg)
            raise RuntimeError(msg)
    def __str__(self): return f"[PROC OS={self.build_info['OS']}, APP={self.build_info['app']} BOARD={self.SN}]"

    def write(self, text: str):
        '''
            @param textRef: Text to write on the board
        '''
        assert(self.port is not None)  # Checked in object construction
        text=f"{self.format}{text}"
        subprocess.run([self.script_path, 'write_protocol', self.port, text], check=True)
        _DEBUG(f"{self} write {text} to {self.port}")
        self.logger.info(f"Command  OK  write : {text}")

    def read(self,  textRef : str, timeout: float):
        '''
            @param textRef: Regular expression to search for in the text read on the board.
            @param timeout: How long the function should wait for the right text
        '''
        assert(self.port is not None )
        read_result = subprocess.run([str(self.script_path), 'read_protocol', self.port, timeout], capture_output=True, text=True, timeout=DEFAULT_SUBPROCESS_TIMEOUT)
        data = read_result.stdout.strip()
        if re.search(textRef, data):
            _DEBUG(f"Data matches the pattern: {textRef}")
            self.logger.info(f"Command  OK read pattern : {textRef}")
        else:
            raise Exception(f"\nData does not match the pattern: {textRef} \nResponse from Board {str(self)} :\n{data}\n")
        return data

    def sleep(self, t):
        '''
            @param t: Duration of the sleep
        '''
        _DEBUG (f"--- Sleep {t}")
        sleep(t)

class Executor:
    def __init__(self, build_config_json :dict, test_key : str, logger, hardware_capa_json):
        try:
            test_cfg = get_json_arg(build_config_json,["tests",test_key])
            builds : list = get_json_arg(test_cfg,["builds"])
            self.test_script : str = get_json_arg(test_cfg,["test_script"])

        except Exception as E:
            raise Exception(f"Bad JSON configuration : {E}" )

        self.logger = logger
        self.hardware_capa_json = hardware_capa_json
        self.procs = {} #  { <name> : remoteProcess }
        for build in builds:
            # build contains {"name":"...", "build_name": "...", "BOARD_SN": ".."},
            test_build_name=get_json_arg(build,["build_name"])
            _DEBUG(build["name"])
            proc = RemoteProcess( get_json_arg(build_config_json,["build",test_build_name]), build, logger, hardware_capa_json)
            proc_name=get_json_arg(build,["name"])
            self.procs[proc_name] = proc
            self.logger.info(f"Testing config : {proc}")
            #_DEBUG(proc)

    def execute (self, script :  list):
        '''
            @param script: a list of ScriptAction that will be executed in array order
        '''
        for action in script: action(self)

def parse_scenario(scenario):
    script_actions = []
    for line in scenario:
        try:
            # line contains ("command","target","args for the command")
            command = line['command']
            target_name = line['target']
            args = line['args']
            if command == "sleep":
                script_actions.append(ScriptActionSleep(target_name, args[0]))
            elif command == "write":
                script_actions.append(ScriptActionWrite(target_name, args[0]))
            elif command == "read":
                timeout, pattern = args
                script_actions.append(ScriptActionRead(target_name, pattern, timeout))
            else :
                raise Exception(f"Unknown scenario command {command}")
        except Exception as E:
            raise Exception(f"scenario_parsing error: {E} , line : {line}" )
    return script_actions
