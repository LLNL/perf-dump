#ifndef PERF_DUMP_H
#define PERF_DUMP_H

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

//
// Following are environment variables that control the way perf_dump works.
//
// Location to put dumps.  Best if this is in a parallel file system.
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


EXTERN_C void pdump_init();

EXTERN_C void pdump_start_step();

EXTERN_C void pdump_end_step();

EXTERN_C void pdump_finalize();

#endif // PERF_DUMP_H
