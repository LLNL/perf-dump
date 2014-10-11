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
#include <mpi.h>
#include <cstdlib>
#include <iostream>
#include <vector>
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
      MPI_Pcontrol(2);

      if (rank == 0) {
         cout << step << ":\t" << prod << endl;
      }
   }

   MPI_Finalize();
}
