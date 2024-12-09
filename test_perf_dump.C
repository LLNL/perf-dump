// Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
// perf-dump Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-Exception
#include <mpi.h>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "perf_dump.h"
using namespace std;

#define MAX_STEP 10
#define VEC_SIZE 1000

void init_vector(vector<double>& vec) {
   for (size_t i=0; i < vec.size(); i++) {
      vec[i] = (double) rand();
   }
}


double dot(const vector<double>& a, const vector<double>& b) {
   double sum = 0;
   for (size_t i=0; i < a.size(); i++) {
      sum += a[i] * b[i];
   }
   return sum;
}


int main(int argc, char **argv) {
   MPI_Init(&argc, &argv);
   pdump_init();

   int rank;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   vector<double> a(VEC_SIZE);
   vector<double> b(VEC_SIZE);

   srand(23578 * rank);
   for (size_t step=0; step < MAX_STEP; step++) {
      pdump_start_step();

      init_vector(a);
      init_vector(b);
      double d = dot(a, b);

      double prod;
      MPI_Reduce(&d, &prod, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
      pdump_end_step();

      if (rank == 0) {
         cout << step << ":\t" << prod << endl;
      }
   }

   pdump_finalize();
   MPI_Finalize();
}
