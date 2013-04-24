#ifndef PERF_DUMP_INTERNAL_H
#define PERF_DUMP_INTERNAL_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>

inline int str_to_int(const std::string& str) {
   char *end_ptr;
   int num = (int) strtol(str.c_str(), &end_ptr, 10);
   if (*end_ptr != '\0') {
      throw std::domain_error("invaild integer: " + str);
   }
}

#endif // PERF_DUMP_INTERNAL_H
