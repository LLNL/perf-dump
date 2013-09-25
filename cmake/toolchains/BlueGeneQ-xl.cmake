# the name of the target operating system
set(CMAKE_SYSTEM_NAME BlueGeneQ-static)

# XL C Compilers
set(CMAKE_C_COMPILER       /opt/ibmcmp/vac/bg/12.1/bin/bgxlc)
set(CMAKE_CXX_COMPILER     /opt/ibmcmp/vacpp/bg/12.1/bin/bgxlC)
set(CMAKE_Fortran_COMPILER /opt/ibmcmp/xlf/bg/14.1/bin/bgxlf90)

# Make sure MPI_COMPILER wrapper matches the gnu compilers.
# Prefer local machine wrappers to driver wrappers here too.
find_program(MPI_COMPILER NAMES mpixlc mpxlc
  PATHS /usr/local/bin /usr/bin /bgsys/drivers/ppcfloor/comm/gcc/bin)
