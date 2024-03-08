#!/bin/bash

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

PUB_BENCH=true
SUB_BENCH=true
DELTA_TIME_PUB=(T1 T2 T3 T4 T5)
DELTA_TIME_SUB=(T1 T2 T3)
PROG_RELATIVEPATH=bench_pubsub
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# can take value "measure" or "analysis"
BENCH_ROLE=
SAVE_IMAGES=false

function usage()
{
    echo "$0 measure [OPTION] [MEASURE_OPTION]"
    echo "Set probes, launch the program and record the timestamp in a *.dat file with trace-cmd"
    echo "To function, this script requires execution with sudo privileges"
    echo ""
    echo "$0 analysis [OPTION] [ANALYSIS_OPTION]"
    echo "Analysis data from benchmark measures with perf"
    echo ""
    echo " OPTION:"
    echo " -dp --delta-pub (T1|T2|T3|T4|T5)     Record probes for a specific publisher process "
    echo " -ds --delta-sub (T1|T2|T3)           Record probes for a specific susbcriber process"
    echo " -p                                   Disable publisher record"
    echo " -s                                   Disable subscriber record"
    echo " -h --help help                       Print this help"
    echo ""
    echo " MEAUSURE_OPTION:"
    echo " -i                                   Path to benchmark program, default ${PROG_RELATIVEPATH}"
    echo ""
    echo " ANALYSIS_OPTION:"
    echo " --save-images                        Save result of analysis in png files"
}

# First argument should match one of following parameters
if [ $1 == "measure" ]; then
  BENCH_ROLE="measure"
  shift
elif [ $1 == "analysis" ]; then
  BENCH_ROLE="analysis"
  shift
elif [ $1 == "h" -o $1 == "help" -o $1 == "-h" -o "--help" ]; then
  usage
  exit 0
else
  echo "First postional argument must be equal to: \"measure\" or \"analysis\""
  exit 1
fi

while [[ $# -gt 0 ]]; do
    case $1 in
    -h|--help|help)
      usage
      exit 0
      ;;
    -dp|--delta-pub)
      shift
      DELTA_TIME_PUB=()
      while [[ $# -gt 0 && ! $1 =~ ^- ]]; do
        DELTA_TIME_PUB+=("$1")
        shift
      done
      for DELTA_T in ${DELTA_TIME_PUB[@]}; do
        case $DELTA_T in
          T1|T2|T3|T4|T5)
            echo "delta time $DELTA_T publisher enabled."
            ;;
          *)
            echo "$DELTA_T is not a valid delta time publisher."
            usage
            exit 1
            ;;
        esac
      done
      ;;
    -ds|--delta-sub)
      shift
      DELTA_TIME_SUB=()
      while [[ $# -gt 0 && ! $1 =~ ^- ]]; do
        DELTA_TIME_SUB+=("$1")
        shift
      done
      for DELTA_T in ${DELTA_TIME_SUB[@]}; do
        case $DELTA_T in
          T1|T2|T3)
            echo "delta time $DELTA_T subscriber enabled."
            ;;
          *)
            echo "$DELTA_T is not a valid delta time subscriber."
            usage
            exit 1
            ;;
        esac
      done
      ;;
    -p)
      echo "Disable publisher"
      PUB_BENCH=false
      shift
      ;;
    -s)
      echo "Disable subscriber"
      SUB_BENCH=false
      shift
      ;;
    -i)
      shift
      PROG_RELATIVEPATH=$1
      echo "Program path set to ${PROG_RELATIVEPATH}"
      shift
      ;;
    --save-images)
      shift
      SAVE_IMAGES=true
      shift
      ;;
    *)
      echo "Unknown option: $1"
      usage
      exit 1
      ;;
  esac
done

# Functions to which attach the probes
T1_PUB_FUNC=(SOPC_UADP_NetworkMessage_Get_PreencodedBuffer MessageCtx_send_publish_message)
T2_PUB_FUNC=(SOPC_PubSourceVariable_GetVariables SOPC_PubSourceVariable_GetVariables%return)
T3_PUB_FUNC=(sendto SOPC_UADP_NetworkMessage_Get_PreencodedBuffer)
T4_PUB_FUNC=(sendto MessageCtx_send_publish_message)
T5_PUB_FUNC=(MessageCtx_send_publish_message MessageCtx_send_publish_message%return)

T1_SUB_FUNC=(Cache_SetTargetVariables Cache_SetTargetVariables%return)
T2_SUB_FUNC=(SOPC_Reader_Read_UADP SOPC_Reader_Read_UADP%return)
T3_SUB_FUNC=(on_socket_message_received on_socket_message_received%return)

# Duration of the benchmark
sampling_time="1m"

function test_set_probe()
{
  # First argument return value from perf probe command
  # Second argument the function we try to add a probe
  PROBE_RETURN=$1
  FUNCTION_NAME=$2
  # 239 exit code means that the event is already set
  test 0 != $PROBE_RETURN && test 239 != $PROBE_RETURN  && echo "Failed to set probe for function $FUNCTION_NAME" && exit 1
}

function set_probe()
{
  PROGPATH=$1
  FUNCTION_NAME=$2
  if [ ${FUNCTION_NAME} == "sendto" ]; then
    perf probe -x /lib/x86_64-linux-gnu/libc.so.6 sendto
  else
    perf probe -x $PROGPATH $FUNCTION_NAME
    test_set_probe $? $FUNCTION_NAME
  fi
}

function del_probe()
{
  # First argument path to program which delete the probes
  PROGPATH=$1
  PROGNAME="$(basename -- $PROGPATH)"

  perf probe --del probe_$PROGNAME:*
  perf probe --del probe_libc:*
}

function set_probe_pub()
{
  # First argument path to program which set the probes
  # Second argument process to measure [T1|T2|T3|T4|T5] accepted
  PROGPATH=$1
  DELTA_TIME=$2
  if [ $DELTA_TIME == "T1" ]; then
    # Process before encoding which includes data gathering from user space
    set_probe $PROGPATH ${T1_PUB_FUNC[0]}
    set_probe $PROGPATH ${T1_PUB_FUNC[1]}
  fi
  if [ $DELTA_TIME == "T2" ]; then
    # Data gathering
    set_probe $PROGPATH ${T2_PUB_FUNC[0]}
    set_probe $PROGPATH ${T2_PUB_FUNC[1]}
  fi
  if [ $DELTA_TIME == "T3" ]; then
    # Encoding process until emmision
    set_probe $PROGPATH ${T3_PUB_FUNC[0]}
    set_probe $PROGPATH ${T3_PUB_FUNC[1]}
  fi
  if [ $DELTA_TIME == "T4" ]; then
    # All the publisher process before emiting
    set_probe $PROGPATH ${T4_PUB_FUNC[0]}
    set_probe $PROGPATH ${T4_PUB_FUNC[1]}
  fi
  if [ $DELTA_TIME == "T5" ]; then
    # Total delay
    set_probe $PROGPATH ${T5_PUB_FUNC[0]}
    set_probe $PROGPATH ${T5_PUB_FUNC[1]}
  fi
}

function set_probe_sub()
{
  # First argument path to program which set the probes
  # Second argument process to measure [T1|T2|T3] accepted
  PROGPATH=$1
  DELTA_TIME=$2
  if [ $DELTA_TIME == "T1" ]; then
    # Cache update delay
    set_probe $PROGPATH ${T1_SUB_FUNC[0]}
    set_probe $PROGPATH ${T1_SUB_FUNC[1]}
  fi
  if [ $DELTA_TIME == "T2" ]; then
    # Packet decoding
    set_probe $PROGPATH ${T2_SUB_FUNC[0]}
    set_probe $PROGPATH ${T2_SUB_FUNC[1]}
  fi
  if [ $DELTA_TIME == "T3" ]; then
    # Total delay between network reception and end of subscriber treatment
    set_probe $PROGPATH ${T3_SUB_FUNC[0]}
    set_probe $PROGPATH ${T3_SUB_FUNC[1]}
  fi
}

function trace_cmd_record()
{
  PROGNAME=$1
  # this list is composed of 2 arguments
  LIST_FUNCTIONS=($2 $3)
  TRACE_OUTPUT=$4

  # Format accepted to record return functions is <function_name>__return instead of <function_name>%return
  LIST_FUNCTIONS_FORMAT=(${LIST_FUNCTIONS[0]//\%return/__return} ${LIST_FUNCTIONS[1]//\%return/__return})

  if [ ${LIST_FUNCTIONS[0]} == "sendto" ]; then
    trace-cmd record -e probe_libc:sendto -e probe_$PROGNAME:${LIST_FUNCTIONS_FORMAT[1]} -o ${TRACE_OUTPUT} &
  else
    trace-cmd record -e probe_$PROGNAME:${LIST_FUNCTIONS_FORMAT[0]} -e probe_$PROGNAME:${LIST_FUNCTIONS_FORMAT[1]} -o ${TRACE_OUTPUT} &
  fi
}

function publisher_measure()
{
  # First argument path to benchmark program
  # Second argument process measured [T1|T2|T3|T4|T5] accepted
  PROGPATH=$1
  PROGNAME="$(basename -- $PROGPATH)"
  RELATIVE_PATH="$(dirname $PROGPATH)"
  DELTA_TIME=$2
  TRACE_OUTPUT="trace_publisher_${DELTA_TIME}.dat"

  if [ $DELTA_TIME == "T1" ]; then
    trace_cmd_record ${PROGNAME} ${T1_PUB_FUNC[@]} ${TRACE_OUTPUT}
  fi
  if [ $DELTA_TIME == "T2" ]; then
    trace_cmd_record ${PROGNAME} ${T2_PUB_FUNC[@]} ${TRACE_OUTPUT}
  fi
  if [ $DELTA_TIME == "T3" ]; then
    trace_cmd_record ${PROGNAME} ${T3_PUB_FUNC[@]} ${TRACE_OUTPUT}
  fi
  if [ $DELTA_TIME == "T4" ]; then
    trace_cmd_record ${PROGNAME} ${T4_PUB_FUNC[@]} ${TRACE_OUTPUT}
  fi
  if [ $DELTA_TIME == "T5" ]; then
    trace_cmd_record ${PROGNAME} ${T5_PUB_FUNC[@]} ${TRACE_OUTPUT}
  fi

  trace_cmd_pid=$!

  cd ${RELATIVE_PATH}
  # wait for recorder to start before launching the program
  sleep 5s
  echo "launch ${PROGNAME} for ${sampling_time}"
  ${PROGPATH} &
  prog_pid=$!

  sleep ${sampling_time}

  kill -s INT $prog_pid
  kill -s INT $trace_cmd_pid

  cd ${SCRIPT_DIR}

  echo "Save new data in ${TRACE_OUTPUT}"
  trace-cmd report -i ${TRACE_OUTPUT}
}

function subscriber_measure()
{
  # First argument path to benchmark program
  # Second argument process measured [T1|T2|T3] accepted
  PROGPATH=$1
  PROGNAME="$(basename -- $PROGPATH)"
  RELATIVE_PATH="$(dirname $PROGPATH)"
  DELTA_TIME=$2
  TRACE_OUTPUT="trace_subscriber_${DELTA_TIME}.dat"

  if [ $DELTA_TIME == "T1" ]; then
    trace_cmd_record ${PROGNAME} ${T1_SUB_FUNC[@]} ${TRACE_OUTPUT}
  fi
  if [ $DELTA_TIME == "T2" ]; then
    trace_cmd_record ${PROGNAME} ${T2_SUB_FUNC[@]} ${TRACE_OUTPUT}
  fi
  if [ $DELTA_TIME == "T3" ]; then
    trace_cmd_record ${PROGNAME} ${T3_SUB_FUNC[@]} ${TRACE_OUTPUT}
  fi

  trace_cmd_pid=$!

  cd ${RELATIVE_PATH}
  # wait for recorder to start before launching the program
  sleep 5s
  echo "launch ${PROGNAME} -k sub for ${sampling_time}"
  ${PROGPATH} -k sub &
  prog_pid=$!

  sleep ${sampling_time}

  kill -s INT $prog_pid
  kill -s INT $trace_cmd_pid

  cd ${SCRIPT_DIR}

  echo "Save new data in ${TRACE_OUTPUT}"
  trace-cmd report -i ${TRACE_OUTPUT}
}

function trace_analysis()
{
  # First parameter defines if it is a publisher or subscriber analysis (pub|sub)
  # Second parameter defines which process is analysed
  # Third parameter optional, Set to save the results as images format "--save-images NAME_IMAGE.png"
  PUSBUB_KIND=$1
  DELTA_TIME=$2
  SAVE_PNG=$3

  if [ $PUSBUB_KIND == "pub" ]; then
    ./parse_trace.py trace_config/traceconfig_publisher_${DELTA_TIME}.yaml ${SAVE_PNG}
  elif [ $PUSBUB_KIND == "sub" ]; then
    ./parse_trace.py trace_config/traceconfig_subscriber_${DELTA_TIME}.yaml ${SAVE_PNG}
  else
    echo "unrecognized PUBSUB_KIND option $PUBSUB_KIND"
    exit 1
  fi
}


if [ $BENCH_ROLE == "measure" ]; then

  test 0 != `id -u` && echo "To function, measurement script requires execution with sudo privileges" && exit 1

  # sanity check on program path
  test ! -e $PROG_RELATIVEPATH && echo "$PROG_RELATIVEPATH does not exist" && exit 2
  test ! -x $PROG_RELATIVEPATH && echo "$PROG_RELATIVEPATH is not executable file" && exit 3

  # If check passed retrieve absolute path to program
  PROGPATH="$(readlink -f $PROG_RELATIVEPATH)"

  if [ $PUB_BENCH == true ]; then
      for DELTA_T in ${DELTA_TIME_PUB[@]}; do
        del_probe ${PROGPATH}
        set_probe_pub ${PROGPATH} ${DELTA_T}
        publisher_measure ${PROGPATH} ${DELTA_T}
      done
  fi


  if [ $SUB_BENCH == true ]; then
      for DELTA_T in ${DELTA_TIME_SUB[@]}; do
        del_probe ${PROGPATH}
        set_probe_sub ${PROGPATH} ${DELTA_T}
        subscriber_measure ${PROGPATH} ${DELTA_T}
      done
  fi
fi

if [ $BENCH_ROLE == "analysis" ]; then
  if [ $PUB_BENCH == true ]; then
      for DELTA_T in ${DELTA_TIME_PUB[@]}; do
        if [ ${SAVE_IMAGES} == true ]; then
          trace_analysis "pub" ${DELTA_T} "--save-images publisher_${DELTA_T}.png"
        else
          trace_analysis "pub" ${DELTA_T}
        fi
      done
  fi


  if [ $SUB_BENCH == true ]; then
      for DELTA_T in ${DELTA_TIME_SUB[@]}; do
        if [ ${SAVE_IMAGES} == true ]; then
          trace_analysis "sub" ${DELTA_T} "--save-images subscriber_${DELTA_T}.png"
        else
          trace_analysis "sub" ${DELTA_T}
        fi
      done
  fi
fi

exit 0
