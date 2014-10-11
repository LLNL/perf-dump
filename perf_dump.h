//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
//
// This file is part of perf-dump.
// Written by Todd Gamblin, tgamblin@llnl.gov, All rights reserved.
// LLNL-CODE-647187
//
// For details, see https://scalability-llnl.github.io/perf-dump
// Please also see the LICENSE file for our notice and the LGPL.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (as published by
// the Free Software Foundation) version 2.1 dated February 1999.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
// conditions of the GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//////////////////////////////////////////////////////////////////////////////
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
