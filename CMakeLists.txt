cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

find_package(MPI REQUIRED)
find_package(PnMPI REQUIRED)
find_package(adept_utils REQUIRED)
find_package(PAPI REQUIRED)

find_package(HDF5 REQUIRED)
if(NOT HDF5_IS_PARALLEL)
  message(FATAL_ERROR
    "This HDF5 is not parallel.  perf_dump requires parallel HDF5.")
endif()

add_library(perfdump SHARED perf_dump.C papi_utils.C)
target_link_libraries(perfdump
  ${MPI_LIBRARIES} adept_utils papi hdf5)

add_wrapped_file(pmpi_perf_dump.C pmpi_perf_dump.w)
add_pnmpi_module(mod_perfdump pmpi_perf_dump.C)
target_link_libraries(mod_perfdump adept_utils perfdump)

add_executable(test-perf-dump test_perf_dump.C)
target_link_libraries(test-perf-dump ${MPI_LIBRARIES} perfdump ${HDF5_LIBRARIES})

include_directories(
  ${MPI_INCLUDE_PATH}
  ${PROJECT_SOURCE_DIR}
  ${HDF5_INCLUDE_DIRS}
  ${adept_utils_INCLUDE_PATH})
