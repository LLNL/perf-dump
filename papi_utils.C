// Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
// perf-dump Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-Exception
#include <iostream>

#include "papi_utils.h"
#include "string_utils.h"

using namespace stringutils;
using namespace std;

PAPIEventSet::PAPIEventSet():
   event_set_(PAPI_NULL)
{
   PAPI_CHECK(PAPI_create_eventset(&event_set_));
}


PAPIEventSet::~PAPIEventSet() {
   PAPI_CHECK(PAPI_cleanup_eventset(event_set_));
   PAPI_CHECK(PAPI_destroy_eventset(&event_set_));
}


void PAPIEventSet::add_event(const string& event_name) {
   PAPI_CHECK(PAPI_add_event(event_set_, name_to_code(event_name)));
   event_names_.push_back(event_name);
   values.push_back(0);
}


int PAPIEventSet::name_to_code(const string& event_name) {
   char *name = const_cast<char*>(event_name.c_str());
   int event;
   PAPI_CHECK(PAPI_event_name_to_code(name, &event));
   return event;
}


void PAPIEventSet::add_from_environment(const string& env_var,
                                        const char *default_events)
{
   const char *event_name_list = getenv(env_var.c_str());
   if (!event_name_list) {
      if (!default_events) {
         return;
      }
      event_name_list = default_events;
   }

   vector<string> env_event_names;
   split(event_name_list, ", ", env_event_names);

   size_t num_counters = (size_t) PAPI_num_hwctrs();
   for (size_t i=0; i < env_event_names.size() && i < num_counters; i++) {
      add_event(env_event_names[i]);
   }

   for (size_t i=num_counters; i < env_event_names.size(); i++) {
      ignored_events_.push_back(env_event_names[i]);
   }
}
