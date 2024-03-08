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

import argparse
import yaml
import numpy as np
from lisa.trace import Trace
import matplotlib.pyplot as plt
from textwrap import wrap
from dataclasses import dataclass, field

@dataclass
class Histogram:
    name : str
    min : np.int64
    max : np.int64
    avg : np.int64
    bin : np.int64
    xaxis : list = field(default_factory=list)
    yaxis : list = field(default_factory=list)

def parse_command_line():
    parser = argparse.ArgumentParser(description="Calculate delta times between probes.")
    parser.add_argument("-p", "--probes", action='store_true', help="show all probes available in the trace file and exit")
    parser.add_argument("--save-images", type=str, help="Save images of the repartition of delta times in format png. Expected argument the name of the png file")
    parser.add_argument('TRACECONFIG', type=str, help='Trace configuration file (YAML format)')

    args = parser.parse_args()

    return args

def realign_timestamps_probes(probe1_timestamps, probe2_timestamps):
    """
        Realign timestamps between probe1 and probe2.
        probe1 occurs after probe2.

        Example:                                  *realignment
                    probe1_timestamps
                    0       71208.409796
                    1       71208.509497
                    2       71208.610161
                    3       71208.709825
                                           --->   Last probe1_timestamp deleted (sizes match)
                    probe2_timestamps:
                    0       71208.308972   --->   Deleted occured before probe1_timestamp 0
                    1       71208.409637          *0
                    2       71208.509394          *1
                    3       71208.610062          *2
    """
    if (probe2_timestamps.iloc[1] < probe1_timestamps.iloc[0]):
        probe2_timestamps = probe2_timestamps.iloc[1:]
        probe2_timestamps.index = range(len(probe2_timestamps.index))
        probe1_timestamps = probe1_timestamps.iloc[0:probe1_timestamps.shape[0]]
    return probe1_timestamps, probe2_timestamps

def show_probes(trace):
    print("\nAvailable probes/events: \n")
    print(trace.available_events)


def analyze_trace(trace_probe1, trace_probe2):
    # Get the timestamps of probe1 and probe2 events
    probe1_timestamps = trace_probe1.reset_index()
    probe2_timestamps = trace_probe2.reset_index()

    probe1_timestamps = probe1_timestamps.iloc[:, 0]
    probe2_timestamps = probe2_timestamps.iloc[:, 0]

    probe1_timestamps, probe2_timestamps = realign_timestamps_probes(probe1_timestamps, probe2_timestamps)

    # Calculate the time difference between corresponding rows
    delta_time = (probe1_timestamps - probe2_timestamps) * 1000000

    # Clean data
    delta_time_cleaned = delta_time[delta_time.notnull()]
    delta_time_cleaned = delta_time_cleaned.round().astype(int)

    return delta_time_cleaned

def show_statistics(histogram):
    print("""  Statistics for %s
    ==   min : %d µs
    ==   max : %d µs
    ==   avg : %d µs
    """ %(histogram.name, histogram.min, histogram.max, histogram.avg))


def format_histogram(df_subset, scale=1):
    # Extract histogram shape
    hist_min = df_subset.min()
    hist_max = df_subset.max()
    hist_avg = round(df_subset.mean())
    hist_bin = (hist_max - hist_min) // scale
    df_subset_occurences = df_subset.value_counts(bins=hist_bin, sort=False)

    # Get x, y axis values
    margin = 1
    yaxis = margin * [0] + df_subset_occurences.tolist() + margin * [0]
    xaxis = np.linspace(hist_min - margin*scale, hist_max + margin*scale, hist_bin + margin*2)

    return Histogram(df_subset.name, hist_min, hist_max, hist_avg, hist_bin, xaxis, yaxis)

def plot_histogram(histogram: Histogram, fig_name):
    plt.step(histogram.xaxis, histogram.yaxis, linewidth=0.8)

    metrics = f"Min: {histogram.min} µs, Max: {histogram.max} µs, Avg: {histogram.avg} µs"
    plt.xlabel('Time elapsed (µs) \n' + metrics)
    x_min = 0
    x_max = 200
    if histogram.avg > 100 :
        x_min = histogram.avg - 100
        x_max = histogram.avg + 100


    plt.xlim([x_min, x_max])

    plt.ylabel('Occurrences')
    plt.yscale('log')

    plt.title('\n'.join(wrap(histogram.name, 60)))
    plt.tight_layout()

    print("Save figure in file " + str(fig_name))
    plt.savefig(str(fig_name))

if __name__ == "__main__":
    args = parse_command_line()
    with open(args.TRACECONFIG, "r") as stream:
        try:
            metadata_trace = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)

    # Get the trace
    trace_probe1 = []
    trace_probe2 = []
    program_name = metadata_trace['program']

    trace = Trace(metadata_trace['filepath'])

    if args.probes:
        show_probes(trace)
        exit(0)

    try:
        for pair in metadata_trace['probes_pair']:
            # iterate other all probes
            trace_probe1 = trace.df_event(pair['probe1'])
            trace_probe2 = trace.df_event(pair['probe2'])

            # check that the program configure correspond with the one record
            trace_probe1 = trace_probe1[trace_probe1['__comm'] == program_name]
            trace_probe2 = trace_probe2[trace_probe2['__comm'] == program_name]
            # Get delta time betwen probe1 and probe2
        deltatime_dataset = analyze_trace(trace_probe1, trace_probe2)

        deltatime_dataset.name = f"Delta time between {pair['probe2']} and {pair['probe1']}"

        histogram = format_histogram(deltatime_dataset)

        show_statistics(histogram)

        if args.save_images != None:
            plot_histogram(histogram, args.save_images)
    except ValueError:
        # This error happen when the script fail to collect the probes.
        # Make sure the probes configured are what you expect.
        # Sometimes measurement fail and no probes are available in the data file, redo measurement
        print("""
   == ERROR : No probes available in %s make sure probes are well configured in file %s
   If probes are well configured just redo measurements ==
""" %(metadata_trace["filepath"], args.TRACECONFIG))
        exit(1)

