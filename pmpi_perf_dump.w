/* -*- C++ -*- */
#include <mpi.h>
#include <iostream>
#include "perf_dump.h"

{{fn fun MPI_Init}} {
  {{callfn}}


  std::cerr << "initing!" << std::endl;

  pdump_init();
  pdump_start_step();
} {{endfn}}


{{fn fun MPI_Pcontrol}} {
  int pcontrol_code = {{0}};
  if (pcontrol_code != PDUMP_PCONTROL_TIME_STEP_CODE) {
    return MPI_SUCCESS;
  }

  pdump_end_step();
  pdump_start_step();

  return MPI_SUCCESS;
} {{endfn}}



{{fn fun MPI_Finalize}} {
  pdump_end_step();
  pdump_finalize();
  {{callfn}}
} {{endfn}}
