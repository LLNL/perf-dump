#!/usr/bin/env python
#
# Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
# perf-dump Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-Exception
"""
Process some time steps from a perf-dump file output from Miranda to print the maximum, average, minimu values.

e.g., for a 256-process Miranda run, you could do this:

  ./miranda-process.py perf-dump.h5 8x32

or you could specify particular time steps:

  ./miranda-process.py perf-dump.h5  -t 0 1 2

The parameters are:
   perf-dump-file    File containing performance data.
   dimensions        dimensions to render, separated by x's.
   Time steps        time steps from the dump file to render.

"""
import argparse
import sys
import numpy as np

import h5py

def die(msg):
    print "Error: " + msg
    sys.exit(1)

parser = argparse.ArgumentParser(description='Open perf-dump files from Miranda.')
parser.add_argument('h5file', help=".h5 file from perf-dump")
parser.add_argument(
    '-t', '--timesteps', dest='timesteps', type=int, nargs='+',
    help='Time steps to process, e.g. 0 1 2 3 or 0 2 4. By default all steps are rendered')
args = parser.parse_args()


def project(step_data_1d, dims):
    """Resizes and converts an array to float"""
    new_arr = np.zeros(dims, dtype=float)
    new_arr.flat = step_data_1d
    return new_arr

def process_dataset(dataset):
    ranks, steps = dataset.shape

    if args.timesteps:
        timesteps = args.timesteps
        if not all(0 <= s < steps for s in timesteps):
            die("All time steps must be in the data set.  Range is [0..%s]"
                % (len(args.steps)-1))
    else:
        timesteps = xrange(steps)

    step_arrays = [project(dataset[:,s], ranks) for s in timesteps]

    for i, arr2 in enumerate(step_arrays):
        mymin = min(arr2)
        mymax = max(arr2)
        mysize = len(arr2)
        mysum = sum(arr2)
        myavg = mysum/mysize
        print "%d" % mysize
        print "step %d    min:avg:max %d:%d:%d ratio(max/avg, max/min) %.2f %.2f " %  (i, mymin, myavg, mymax, mymax/myavg, mymax/mymin)


h5file = h5py.File(args.h5file)
for name, dataset in zip(h5file.iternames(), h5file.itervalues()):
    print "\n---------metrics for %s---------"  % name
    process_dataset(dataset)
h5file.close()
