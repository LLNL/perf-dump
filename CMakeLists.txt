cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

find_package(MPI REQUIRED)
find_package(PnMPI QUIET)
find_package(adept_utils REQUIRED)
find_package(PAPI REQUIRED)

set(WRAP ${WRAP_DIR}/wrap.py)
include(${WRAP_DIR}/WrapConfig.cmake)

find_package(HDF5 REQUIRED)
if(NOT HDF5_IS_PARALLEL)
  message(FATAL_ERROR
    "This HDF5 is not parallel.  perf_dump requires parallel HDF5.")
endif()

set(PERF_DUMP_SRCS perf_dump.C papi_utils.C)

add_library(perfdump SHARED ${PERF_DUMP_SRCS})
target_link_libraries(perfdump
  ${MPI_LIBRARIES} adept_utils ${PAPI_LIBRARIES} ${HDF5_LIBRARIES})

add_wrapped_file(pmpi_perf_dump.C pmpi_perf_dump.w)

add_library(pmpi_perfdump        SHARED pmpi_perf_dump.C ${PERF_DUMP_SRCS})
add_library(pmpi_perfdump_static STATIC pmpi_perf_dump.C ${PERF_DUMP_SRCS})

if(PnMPI_FOUND)
  add_pnmpi_module(pnmpi_perfdump pmpi_perf_dump.C ${PERF_DUMP_SRCS})
  target_link_libraries(pnmpi_perfdump)
endif()

add_executable(test-perf-dump test_perf_dump.C)
target_link_libraries(test-perf-dump
  ${PAPI_LIBRARIES} perfdump ${HDF5_LIBRARIES} ${MPI_LIBRARIES})

add_executable(test-perf-dump-pmpi test_perf_dump_pmpi.C)
target_link_libraries(test-perf-dump-pmpi
  pmpi_perfdump_static ${PAPI_LIBRARIES}
  ${HDF5_LIBRARIES} adept_utils ${MPI_LIBRARIES})

include_directories(
  ${MPI_INCLUDE_PATH}
  ${PROJECT_SOURCE_DIR}
  ${HDF5_INCLUDE_DIRS}
  ${PAPI_INCLUDE_DIRS}
  ${adept_utils_INCLUDE_PATH})
