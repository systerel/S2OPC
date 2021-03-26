#!/bin/bash -xe

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


# This script tests multiple publishing intervals for the pubsub sample with cache

# We want to record:
# - hostname or friendly name
# - rtt traces
# - %CPU usage (flame graph changes measurements)

# We have to do:
# - generate the configurations
# - start the loopback on some machine (may be this one)
# - start the emitter on this machine, configure the number of samples and save file
# - perf stat the emitter, at least longer than samples*freq
# - kill the emitter
# - kill the loopback

# We expect that:
# - ssh is configured to connect to the remote with "ssh REMOTE_HOST"
# - the bin dir is on the other machine
# - pubsub is owned by root and the setuid bit is set (sudo chown root pubsub && sudo chmod u+s pubsub)


# We have to be sudoer for perf stat
echo "Were you sudoer recently?"
sudo echo "Thanks!"

# Empty REMOTE_HOST means no ssh
if [[ ! -z "${REMOTE_HOST}" && -z "${REMOTE_PATH}" ]]; then
    printf "REMOTE_HOST=%s but REMOTE_PATH unset\n" "${REMOTE_HOST}"
    exit 1
fi

sleep_time=30
work_dir=$(mktemp -d)
list_pubints_ms=("100" "30"   "10"   "3"     "1"    "0.3"   "0.1"   "0.03")
#list_n_samples=( "256" "1024" "2048" "4096" "8192" "32768" "65536" "65536")
# Compute the number of samples to cover the sleep_time
list_n_samples=( "300" "1000" "3000" "10000" "30000" "100000" "300000" "1000000")
list_names=(    "100m" "30m"  "10m"  "3m"    "1m"   "300µ"  "100µ"  "30µ")
list_secmodes="none signAndEncrypt"

# Generate configurations
for secmode in ${list_secmodes}; do
    for i in "${!list_names[@]}"; do
        output_emit="${work_dir}/config_rtt_emit_${secmode}_${list_names[i]}.xml"
        output_loop="${work_dir}/config_rtt_loop_${secmode}_${list_names[i]}.xml"
        printf "Gen %s & %s\n" ${output_emit} ${output_loop}
        ./gen_xmls.py ${list_pubints_ms[i]} ${secmode} --output-emitter ${output_emit} --output-loopback ${output_loop}
    done
done

# If remote, copy to a temp dir there
if [[ ! -z "${REMOTE_HOST}" ]]; then
    remote_work_dir=$(ssh ${REMOTE_HOST} "mktemp -d")
    printf "Working on remote in %s\n" ${remote_work_dir}
    scp "${work_dir}/config_rtt_loop_"*.xml ${REMOTE_HOST}:"${remote_work_dir}"
    rm "${work_dir}/config_rtt_loop_"*.xml
fi

# Do the experiments
for secmode in ${list_secmodes}; do
    for i in "${!list_names[@]}"; do
        printf "###############\nTest %d samples @ %ss, secu is %s\n" ${list_n_samples[i]} ${list_names[i]} ${secmode}
        # Start loopback
        if [[ -z "${REMOTE_HOST}" ]]; then
            config_loop="${work_dir}/config_rtt_loop_${secmode}_${list_names[i]}.xml"
            env IS_LOOPBACK=1 PUBSUB_XML_CONFIG="${config_loop}" ./pubsub &
            pid_loopback=$!
        else
            config_loop="${remote_work_dir}/config_rtt_loop_${secmode}_${list_names[i]}.xml"
            pubsub_out="${remote_work_dir}/pubsub_${secmode}_${list_names[i]}.out"
            # TODO: pipe this into local grep...
            pid_loopback=$((ssh ${REMOTE_HOST} | tee -a /dev/tty | grep -Po "Loopback pid is \K\d+") <<- EOF
				cd ${REMOTE_PATH}
				nohup env IS_LOOPBACK=1 PUBSUB_XML_CONFIG="${config_loop}" ./pubsub < /dev/null > "${pubsub_out}"  &
				echo "Loopback pid is \$!"
			EOF
            )
        fi
        # Start emitter
        config_emit="${work_dir}/config_rtt_emit_${secmode}_${list_names[i]}.xml"
        csv_prefix="${work_dir}/rtt_emit_${secmode}_${list_names[i]}"
        env RTT_SAMPLES="${list_n_samples[i]}" PUBSUB_XML_CONFIG="${config_emit}" CSV_PREFIX="${csv_prefix}" ./pubsub &
        pid_emitter=$!
        # Stat it
        pubsub_out="${work_dir}/pubsub_${secmode}_${list_names[i]}.stat"
        # perf stat requires sudo on most platform to snoop into other pids
        # perf stat prints its number according to locale, don't do this (with LANG=)
        # perf stat -e duration_time is only available on Linux 5.2+ so don't count on it
        #  (even though it would have been easier to use -x which produces a CSV)
        LANG= sudo perf stat -p ${pid_emitter} --no-big-num -- sleep ${sleep_time} |& tee ${pubsub_out}
        # Kill emitter then the loopback
        kill ${pid_emitter}
        if [[ -z "${REMOTE_HOST}" ]]; then
            kill ${pid_loopback}
            sleep 1.
        else
            ssh ${REMOTE_HOST} kill ${pid_loopback}
        fi
        while ps -p ${pid_emitter} || ps -p ${pid_loopback} ; do
            sleep 1.
        done
    done
done

# Do the analyzes
report=${work_dir}/report.md
for i in "${!list_names[@]}"; do
    printf "### Publish interval ${list_names[i]}s\n\n" >> ${report}
    for secmode in ${list_secmodes}; do
        printf "#### ${secmode}\n\n" >> ${report}
        csv_prefix="${work_dir}/rtt_emit_${secmode}_${list_names[i]}"
        ./analyze_rtt.py "${csv_prefix}"-*.csv >> ${report}
        printf "\n\n" >> ${report}
    done
done

cat ${report}
