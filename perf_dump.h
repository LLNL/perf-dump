// Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
// perf-dump Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-Exception
#ifndef PERF_DUMP_H
#define PERF_DUMP_H

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

// ---------------------------------------------------------------------------
// Environment variables that control the way perf_dump works.
// ---------------------------------------------------------------------------

// Location to put dumps.  Best if this is in a parallel file system.
// Default location is current working directory.
#define PDUMP_DUMP_DIR "PDUMP_DUMP_DIR"

// Comma-separated list of PAPI event names to monitor
#define PDUMP_EVENTS "PDUMP_EVENTS"

// Comma-separated list of timestep numbers to dump data for.
// Default is to dump all steps. First timestep is 0.
#define PDUMP_DUMP_STEPS "PDUMP_DUMP_STEPS"

// Chunk size for timestep dimension for stored files
#define PDUMP_TIME_CHUNK "PDUMP_TIME_CHUNK"


// ---------------------------------------------------------------------------
// Below are default settings for the above environment variables.
// ---------------------------------------------------------------------------

// Default value for PDUMP_EVENTS.
#define PDUMP_DEFAULT_EVENTS  "PAPI_L1_TCM"

// Default value for PDUMP_TIME_CHUNK
#define PDUMP_DEFAULT_TIME_CHUNK 32


// ---------------------------------------------------------------------------
// Pcontrol semantics for PMPI perf dump
// ---------------------------------------------------------------------------

/// MPI_Pcontrol(2) indicates that we should stop current step
/// and start a new one.
#define PDUMP_PCONTROL_TIME_STEP_CODE 2


EXTERN_C void pdump_init();

EXTERN_C void pdump_start_step();

EXTERN_C void pdump_end_step();

EXTERN_C void pdump_finalize();

#endif // PERF_DUMP_H
