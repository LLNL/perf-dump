#!/usr/bin/env python
"""
Plot some time steps from a perf-dump file output from Miranda.

e.g., for a 256-process Miranda run, you could do this:

  ./miranda-plot.py perf-dump.h5 8x32

or you could specify particular time steps:

  ./miranda-plot.py perf-dump.h5 8x32 -t 0 1 2

The parameters are:
   perf-dump-file    File containing performance data.
   dimensions        dimensions to render, separated by x's.
   Time steps        time steps from the dump file to render.

This will pop up windows for each counter in the dump, plotted on
an 8x32 grid, for steps 0, 1, 2, and 3.

"""
import argparse
import sys
import numpy as np

import matplotlib.pyplot as plt
import matplotlib.cm as cm
import h5py

def die(msg):
    print "Error: " + msg
    sys.exit(1)

parser = argparse.ArgumentParser(description='Open perf-dump files from Miranda.')
parser.add_argument('h5file', help=".h5 file from perf-dump")
parser.add_argument('dims', help='Dimensions to project the matrix onto, e.g. 2x2')
parser.add_argument(
    '-t', '--timesteps', dest='timesteps', type=int, nargs='+',
    help='Time steps to plot, e.g. 0 1 2 3 or 0 2 4. By default all steps are rendered')
args = parser.parse_args()


def project(step_data_1d, dims):
    """Resizes and converts an array to float"""
    new_arr = np.zeros(dims, dtype=float)
    new_arr.flat = step_data_1d
    return new_arr

def heat_map(axes, arr, color_map):
    axes.imshow(arr, cmap=color_map, interpolation='nearest')
    nrows, ncols = arr.shape
    def format_coord(x, y):
        col = int(x+0.5)
        row = int(y+0.5)
        if col >= 0 and col < ncols and row >= 0 and row < nrows:
            z = arr[row,col]
            return 'x=%1.4f, y=%1.4f, z=%1.4f'%(x, y, z)
        else:
            return 'x=%1.4f, y=%1.4f'%(x, y)
    axes.format_coord = format_coord



def plot_dataset(dataset):
    ranks, steps = dataset.shape

    if args.timesteps:
        timesteps = args.timesteps
        if not all(0 <= s < steps for s in timesteps):
            die("All time steps must be in the data set.  Range is [0..%s]"
                % (len(args.steps)-1))
    else:
        timesteps = xrange(steps)

    dims = [int(d) for d in args.dims.split('x')]
    if reduce(lambda x,y: x*y, dims, 1) != ranks:
        die("Product of dimensions must equal number of ranks.")

    step_arrays = [project(dataset[:,s], dims) for s in timesteps]

    fig = plt.figure()
    for i, arr in enumerate(step_arrays):
        ax = fig.add_subplot(1, len(step_arrays), i)
        heat_map(ax, arr, cm.Blues)

    fig.canvas.set_window_title(dataset.name)
    fig.show()


h5file = h5py.File(args.h5file)
for dataset in h5file.itervalues():
    plot_dataset(dataset)

plt.show()
h5file.close()
