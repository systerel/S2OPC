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


# Simple script to analyze what's been output by the emitter pubsub

import argparse
import re
import sys
from pathlib import Path
from collections import namedtuple

from tabulate import tabulate  # Optional


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

Stats = namedtuple('Stats', ['min', 'max', 'mean', 'deviation', 'missed', 'pct_missed'])
def do_stats(xs):
    # ys is xs without nulls, which are misses
    ys = [x for x in xs if x != 0]
    m = min(ys) if ys else 0
    M = max(ys) if ys else 0
    mean = sum(ys)/len(ys) if ys else 0
    # This way of computing the std dev is not the most accurate but it yields correct-enough results
    std_dev = (sum((y-mean)**2 for y in ys)/len(ys))**.5 if ys else 0
    missed = sum(1 for x in xs if x == 0)  # should also be len(xs)-len(ys)
    frac_missed = missed/len(xs)
    return Stats(m, M, mean, std_dev, missed, 100*frac_missed)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='TODO')
    parser.add_argument('rtt_fname', metavar='RTT.CSV')
    args = parser.parse_args()

    # Find the publishing interval + secmode + stat file from CSV's path
    #/tmp/tmp.ldLNeu1OQK/rtt_emit_signAndEncrypt_3m-*.csv
    #/tmp/tmp.ldLNeu1OQK/pubsub_signAndEncrypt_3m.stat
    reSecModePubInt = re.compile(r'rtt_emit_(none|sign|signAndEncrypt)_(\d+[mµ])-.+\.csv')
    m = reSecModePubInt.search(args.rtt_fname)
    if m is None:
        eprint('Unknown rtt_fname format', args.rtt_fname)
        sys.exit(1)

    sec_mode, pub_int = m.groups()
    pub_int_ms = int(pub_int[:-1]) / ({'m': 1, 'µ': 1000}[pub_int[-1]])

    # Get the measures from the stat file
    stat_fname = Path(args.rtt_fname).with_name('pubsub_{}_{}.stat'.format(sec_mode, pub_int))
    task_clock = None
    duration = None
    reNumber = re.compile(r'(\d+\.\d+)')
    for line in open(str(stat_fname)):
        if 'task-clock' in line and 'msec' in line:
            task_clock, *_ = reNumber.findall(line)
        if 'seconds time elapsed' in line:
            duration, *_ = reNumber.findall(line)
    if task_clock is None or duration is None:
        eprint('Did not find numbers in stat file', stat_fname)
        sys.exit(2)

    conso_ms = float(task_clock)
    tot_s = float(duration)

    print('Experiment consumed on average {:.2f}% CPU\n'.format(conso_ms/10/tot_s))

    # Get the timings from the csv
    emit_times = []
    rtts = []
    with open(args.rtt_fname) as csv:
        # Drop the headers
        lines = iter(csv)
        for _ in range(5):
            next(csv)
        # Fill the lists
        for line in lines:
            emit_time, rtt = map(int, line.strip().split(';'))
            # Remove incomplete results (if we stop too soon, the buffer is not filled entirely, and there are trailing zeros)
            if emit_time == 0 and rtt == 0:
                continue
            emit_times.append(emit_time)
            rtts.append(rtt)

    # For emit times, do stat on the deltas between two emissions
    #  (and convert to µs)
    emit_stats = do_stats([(b-a)/1000 for a,b in zip(emit_times, emit_times[1:])])
    rtt_stats = do_stats([rtt/1000 for rtt in rtts])

    print('Statistics on {} measures ({} emission deltas):\n\n'.format(len(rtts), len(emit_times)-1))

    headers = ['', 'Min (µs)', 'Max (µs)', 'Mean (µs)', 'Std Dev (µs)', 'Missed', '% Missed']
    dats = [['Emit'] + list(emit_stats), ['Rtt'] + list(rtt_stats)]
    print(tabulate(dats, headers, tablefmt='github'))  # grid for pandoc
