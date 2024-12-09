Performance Dump Module (perf-dump)
============================================

`libperfdump` is a tool for collecting counter data from all processes in a
cluster and dumping counter values out to an HDF5 file with parallel I/O.

by Todd Gamblin, tgamblin@llnl.gov.

Building
-----------------
Building with CMake is very simple, but you will need wrap.py, PAPI and HDF5.
These are installed on the LLNL Linux clusters. Try this:

    export HDF5=/usr/local/tools/hdf5-intel-parallel-mvapich2-1.8.16
    export CMAKE_PREFIX_PATH=$HDF5
    cmake -DWRAP_DIR=<path-to-wrap> -DPAPI_PREFIX=/usr/local/tools/papi  ..
    make

If you do not set the CMAKE_PREFIX_PATH like this, FindHDF5 seems to prefer
the builtin sequential HDF5 (i.e., no MPI), so you should be sure to set
the HDF5 prefix this way.  PAPI should also be found automatically.

Building on Blue Gene/Q
-----------------
For Blue Gene builds, you need to use a toolchain file and you need a version
of CMake with Blue Gene/Q support.  We provide toolchain files that tell CMake
about the Blue Gene/Q cross-compilers.  They are in cmake/toolchains.  To build
on Blue Gene with GNU compilers, build like this:

    HDF5=/usr/local/tools/hdf5/hdf5-1.8.5/parallel
    export CMAKE_PREFIX_PATH=$HDF5
    cmake \
        -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/BlueGeneQ-xl.cmake \
        -DWRAP_DIR=<path-to-wrap> -DPAPI_PREFIX=/usr/local/tools/papi \
        ..

Usage
-----------------

There are actually two ways to use `perf_dump`.  They are:

1. As an API
2. As a PMPI or PnMPI module

### perf-dump API
Here's example usage of perf_dump directly from really simple MPI program:

    int main(int argc, char **argv) {
       MPI_Init(&argc, &argv);
       pdump_init();     // init perf_dump library

       int rank;
       MPI_Comm_rank(MPI_COMM_WORLD, &rank);

       vector<double> a(VEC_SIZE);
       vector<double> b(VEC_SIZE);

       srand(23578 * rank);
       for (size_t step=0; step < MAX_STEP; step++) {
          pdump_start_step();   // start a time step, starting counters

          init_vector(a);
          init_vector(b);
          double d = dot(a, b);

          double prod;
          MPI_Reduce(&d, &prod, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
          pdump_end_step();     // end a time step and stop counters

          if (rank == 0) {
             cout << step << ":\t" << prod << endl;
          }
       }

       // finish writing out performance data to disk
       pdump_finalize();
       MPI_Finalize();
    }

### perf-dump PMPI or PnMPI module
If you do not want to link to the perf-dump library directly, you can
instrument your program with `MPI_Pcontrol(2)` calls to make it work
transparently.  Here is an example of that:

    int main(int argc, char **argv) {
       MPI_Init(&argc, &argv);

       int rank;
       MPI_Comm_rank(MPI_COMM_WORLD, &rank);

       vector<double> a(VEC_SIZE);
       vector<double> b(VEC_SIZE);

       srand(23578 * rank);
       for (size_t step=0; step < MAX_STEP; step++) {
          init_vector(a);
          init_vector(b);
          double d = dot(a, b);

          double prod;
          MPI_Reduce(&d, &prod, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
          MPI_Pcontrol(2);     // end last time step and start a new one.

          if (rank == 0) {
             cout << step << ":\t" << prod << endl;
          }
       }

       // finish writing out performance data to disk
       MPI_Finalize();
    }

Here, `MPI_Init` does the job of calling `pdump_init` and the first
`pdump_start_step`.  `MPI_Pcontrol(2)` tells the tool library to call
`pdump_end_step` to end the previous time step and `pdump_start_step`
to start a new one.  `MPI_Finalize` will call `pdump_end_step` to end
the last time step and it will call `pdump_finalize` to dump any
remaining data to disk.

### Output of perf-dump

Both examples above will record counter values for each time step and
write them to a dataset in an hdf5 file called `perf-dump.h5`.

If you run either of the above programs (which you can find in
`test-perf-dump.C` and test-perf-dump-pmpi.C), the output should look
like this:

    $ env PDUMP_EVENTS=PAPI_L1_TCM srun -n 4 -ppdebug ./test-perf-dump
    ============== perf_dump module started ============
      Initialized PAPI with 1 events:
          PAPI_L1_TCM
    ====================================================
    0:      4.60027e+21
    1:      4.67344e+21
    2:      4.6825e+21
    3:      4.61943e+21
    4:      4.60479e+21
    5:      4.54754e+21
    6:      4.56227e+21
    7:      4.52928e+21
    8:      4.51068e+21
    9:      4.60068e+21

This will produce a file with a data set for PAPI_L1_TCM counts.  The
dataset is stored in HDF5 format and might look kind of like this:

    $ h5dump ./perf-dump.h5
    HDF5 "./perf-dump.h5" {
    GROUP "/" {
       DATASET "PAPI_L1_TCM" {
          DATATYPE  H5T_STD_I64LE
          DATASPACE  SIMPLE { ( 4, 11 ) / ( 4, H5S_UNLIMITED ) }
          DATA {
          (0,0): 1032, 46912530456296, 32, 65, 3925982895, 12884901892,
          (0,6): 17179869188, 4, 9498448, 0, 134,
          (1,0): 305, 3926048431, 12884901892, 9498448, 25769803782, 0, 1,
          (1,7): 9498640, 9498640, 9499008, 4294967431,
          (2,0): 0, 9471688, 515396075520, 0, 0, 0, 0, 46912501089472, 9499008,
          (2,9): 0, 0,
          (3,0): 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
          }
       }
    }
    }

For more on HDF5, see the HDF Group's website: http://www.hdfgroup.org/HDF5/.

You can load this dataset into many visualization and data analysis packages to
examine the data you have collected.

Configuration
------------------------

In `perf_dump.h`, there are a number of environment variables you can set to change
the behavior of perf_dump:

    // Location to put dumps.  Best if this is in a parallel file system.
    // Default location is in the working directory.
    #define PDUMP_DUMP_DIR "PDUMP_DUMP_DIR"

    // List of PAPI event names to monitor, separated by ':'
    #define PDUMP_EVENTS "PDUMP_EVENTS"

    // Comma-separated list of timestep numbers to dump data for.
    // First timestep is 0.
    #define PDUMP_DUMP_STEPS "PDUMP_DUMP_STEPS"

    // Default value for PDUMP_EVENTS.
    #define PDUMP_DEFAULT_EVENTS  "PAPI_L1_TCM"

    // Chunk size for timestep dimension for stored files
    #define PDUMP_TIME_CHUNK "PDUMP_TIME_CHUNK"

If you want good performance, you probably want to point `PDUMP_DUMP_DIR` a the
parallel filesystem like `/p/lscratcha`, e.g.:

    export PDUMP_DUMP_DIR=/p/lscratcha/gamblin2/dumps

If you want to measure multiple events, you can customize PDUMP_EVENTS, e.g.:

    export PDUMP_EVENTS="PAPI_L1_TCM,PAPI_BR_UCN"


Types of builds
------------------------

In this project, there are builds three versions of libperfdump:

1. `libperfdump.so`
2. `libpmpi_perfdump.so`
2. `pnmpi_perfdump.so`

The first is a library with C linkage that lets the user call functions.  This is
the simplest one to use.  The second is a PMPI library, which can be linked
with an MPI program so that perf_dump is inited and finalized automatically.
The third is for PnMPI, and can be loaded as a PnMPI module.



Release
-------------

`perf-dump` is released under the Apache-2.0 license with LLVM exceptions. For more
details see the `LICENSE` file.

SPDX-License-Identifier: Apache-2.0 WITH LLVM-Exception

`LLNL-CODE-2001501`
