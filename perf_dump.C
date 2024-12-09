// Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
// perf-dump Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-Exception
#include <string>
#include <set>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include <mpi.h>
#include <hdf5.h>

#include "perf_dump.h"
#include "papi_utils.h"
#include "string_utils.h"
#include "io_utils.h"

using namespace std;
using namespace stringutils;
using namespace io_utils;

// Number of time steps encountered so far.
static int step_count = 0;

// timestep numbers of steps to dump out.
static set<int> dump_steps;

// PAPI event set for recording counters
static PAPIEventSet *event_set = NULL;
static vector<long long> event_values;
static const hid_t H5_COUNTER_TYPE = H5T_NATIVE_LLONG;

static string dump_file_name("");


static int str_to_int(const std::string& str) {
  char *end_ptr;
  int num = (int) strtol(str.c_str(), &end_ptr, 10);
  if (*end_ptr != '\0') {
    throw std::domain_error("invaild integer: " + str);
  }
}


static void init_dump_steps() {
  int rank;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const char *dump_steps_var = getenv(PDUMP_DUMP_STEPS);
  if (!dump_steps_var) {
    return;
  }

  vector<string> steps;
  split(dump_steps_var, ", ", steps);

  try {
    transform(steps.begin(), steps.end(),
              inserter(dump_steps, dump_steps.end()),
              str_to_int);
  } catch (domain_error& e) {
    if (rank == 0) cerr << e.what() << endl;
    PMPI_Abort(MPI_COMM_WORLD, 1);
  }
}


static string get_dump_file(const string& name) {
  ostringstream filename;

  const char *dump_dir = getenv(PDUMP_DUMP_DIR);
  if (dump_dir) {
    string dump_dir_str(dump_dir);
    if (!dump_dir_str.empty()) {
      filename << dump_dir_str;
      size_t back = dump_dir_str.size() - 1;
      if (dump_dir_str[back] != '/') {
        filename << "/";
      }
    }
  }

  filename << name << ".h5";
  return filename.str();
}


static hid_t create_dump_file(const string& filename, MPI_Comm comm) {
  int nprocs;
  PMPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  hsize_t time_chunk_size = PDUMP_DEFAULT_TIME_CHUNK;
  const char *time_chunk = getenv(PDUMP_TIME_CHUNK);
  if (time_chunk) {
    time_chunk_size = str_to_int(time_chunk);
  }

  hid_t access_plist = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(access_plist, MPI_COMM_WORLD, MPI_INFO_NULL);
  hid_t h5file = H5Fcreate(
    filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, access_plist);

  hsize_t cur_dims[]   = {nprocs, 1};
  hsize_t chunk_dims[] = {nprocs, time_chunk_size};
  hsize_t max_dims[]   = {nprocs, H5S_UNLIMITED};
  hsize_t ndim = sizeof(max_dims) / sizeof(hsize_t);

  const vector<string>& event_names = event_set->event_names();
  for (size_t i=0; i < event_names.size(); i++) {
    hid_t space   = H5Screate_simple(ndim, cur_dims, max_dims);

    hid_t chunk_plist = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(chunk_plist, ndim, chunk_dims);
    hid_t dset = H5Dcreate(h5file, event_names[i].c_str(), H5_COUNTER_TYPE,
                           space, H5P_DEFAULT, chunk_plist, H5P_DEFAULT);

    H5Pclose(chunk_plist);
    H5Sclose(space);
    H5Dclose(dset);
  }

  H5Pclose(access_plist);
  return h5file;
}


static hid_t open_dump_file(const string& filename, MPI_Comm comm) {
  hid_t access_plist = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(access_plist, comm, MPI_INFO_NULL);
  hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDWR, access_plist);
  H5Pclose(access_plist);

  return file;
}


void append_row(hid_t dump_file_id, const string& event_name,
                const long long *data, MPI_Comm comm) {
  int rank, nprocs;
  PMPI_Comm_rank(comm, &rank);
  PMPI_Comm_size(comm, &nprocs);

  hid_t dataset_id = H5Dopen1(dump_file_id, event_name.c_str());
  hid_t space_id   = H5Dget_space(dataset_id);

  hsize_t ndim = H5Sget_simple_extent_ndims(space_id);
  assert(ndim == 2);
  hsize_t dims[ndim], maxdims[ndim];
  H5Sget_simple_extent_dims(space_id, dims, maxdims);

  dims[1] += 1;  // add another time step
  H5Dset_extent(dataset_id, dims);

  hsize_t start[]  = {rank, dims[1] - 2};
  hsize_t marray[] = {1};

  hid_t xfer_plist = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);

  hid_t mid = H5Screate_simple(1, marray, NULL);


  herr_t ret = H5Sselect_elements (space_id, H5S_SELECT_SET, 1, start);
  herr_t status = H5Dwrite(dataset_id, H5_COUNTER_TYPE, mid, space_id,
                           xfer_plist, data);
  H5Dclose(dataset_id);
  H5Sclose(space_id);
  H5Pclose(xfer_plist);
}


// Dump out dataviz information to potentially multipe partial
// dump files.
static void dump(MPI_Comm comm) {
  if (!dump_steps.empty() && !dump_steps.count(step_count)) {
    return;
  }

  hid_t dump_file_id;
  if (dump_file_name.empty()) {
    dump_file_name = get_dump_file("perf-dump");
    dump_file_id = create_dump_file(dump_file_name, comm);
  } else {
    dump_file_id = open_dump_file(dump_file_name, comm);
  }

  const vector<string>& event_names = event_set->event_names();
  for (size_t i=0; i < event_names.size(); i++) {
    append_row(dump_file_id, event_names[i], &event_set->values[i], comm);
  }

  H5Fclose(dump_file_id);
}


// Print out elements of some iterable thing to the output stream
// as a comma-separated list.
template <class Iterable>
static void print_comma_separated(Iterable elements, ostream& out) {
  typename Iterable::const_iterator iter = elements.begin();

  if (iter != elements.end()) {
    out << *iter++;
  }

  while (iter != elements.end()) {
    out << ", " << *iter++;
  }
}


//
// Below are external API functions.
// TODO: allow environment options to be set from PnMPI arguments as well.
//
EXTERN_C void pdump_init() {
  int rank, size;
  PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  PMPI_Comm_size(MPI_COMM_WORLD, &size);

  PAPI_library_init(PAPI_VER_CURRENT);
  event_set = new PAPIEventSet();
  event_set->add_from_environment(PDUMP_EVENTS, PDUMP_DEFAULT_EVENTS);

  const vector<string>& event_names = event_set->event_names();

  if (rank == 0) {
    cerr << "============== perf_dump module started ============" << endl;
    cerr << "  Initialized PAPI with " << event_names.size() << " events:"
         << endl;
    cerr << "      ";
    print_comma_separated(event_names, cerr);
    cerr << endl;
    if (!dump_steps.empty()) {
      cerr << "  Dump timesteps:" << endl;
      cerr << "      ";
      print_comma_separated(dump_steps, cerr);
      cerr << endl;
    }
    cerr << "====================================================" << endl;
  }
}

EXTERN_C void pdump_start_step() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  event_set->start();
}


EXTERN_C void pdump_end_step() {
  event_set->stop();
  dump(MPI_COMM_WORLD);
  step_count++;

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}


EXTERN_C void pdump_finalize() {
  if (event_set) {
    delete event_set;
  }
}

// Wrapper functions for fortran binding
EXTERN_C void PDUMP_INIT() {
  pdump_init();
}

EXTERN_C void pdump_init_() {
  pdump_init();
}

EXTERN_C void pdump_init__() {
  pdump_init();
}

EXTERN_C void PDUMP_START_STEP() {
  pdump_start_step();
}

EXTERN_C void pdump_start_step_() {
  pdump_start_step();
}

EXTERN_C void pdump_start_step__() {
  pdump_start_step();
}

EXTERN_C void PDUMP_END_STEP() {
  pdump_end_step();
}

EXTERN_C void pdump_end_step_() {
  pdump_end_step();
}

EXTERN_C void pdump_end_step__() {
  pdump_end_step();
}

EXTERN_C void PDUMP_FINALIZE() {
  pdump_finalize();
}

EXTERN_C void pdump_finalize_() {
  pdump_finalize();
}

EXTERN_C void pdump_finalize__() {
  pdump_finalize();
}
