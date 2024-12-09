// Copyright 2013-2024 Lawrence Livermore National Security, LLC and other
// perf-dump Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-Exception
#ifndef PAPI_UTILS_H
#define PAPI_UTILS_H

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

#include "papi.h"

/// Use this on a PAPI call to automatically make sure its result is PAPI_OK
/// If the call fails this will print the file and the line along with a useful
/// abort message.
#define PAPI_CHECK(papi_call)                                                 \
   do {                                                                       \
      int papi_code = papi_call;                                              \
      if (papi_code != PAPI_OK) {                                             \
         std::fprintf(stderr, "ERROR: %s (%d) at %s:%d\n",                    \
                      PAPI_strerror(papi_code), papi_code,                    \
                      __FILE__, __LINE__);                                    \
         std::abort();                                                        \
      }                                                                       \
   } while (false)


///
/// Thin wrapper around PAPI's event set API.  Makes it easy to get some events
/// from the environment and add them to an event set.  This handles all the
/// name to code translation and keeps track of the particular events beng
/// monitored.
///
class PAPIEventSet {
public:
   /// Create an empty event set.
   PAPIEventSet();

   /// Free the underlying event set.
   ~PAPIEventSet();

   /// Add an event to this event set by name.
   void add_event(const std::string& event_name);

   /// Remove an event from this event set.
   void remove_event(const std::string& event_name);

   /// Start the counters in this event set.
   void start() {
      PAPI_CHECK(PAPI_start(event_set_));
   }

   /// Stop the counters in this event set and put their
   /// values into this->values.
   void stop() {
      PAPI_CHECK(PAPI_stop(event_set_, &values[0]));
   }

   /// Stop the counters in this event set and put their
   /// values into a custom array supplied by the user.
   void stop(long long *values) {
      PAPI_CHECK(PAPI_stop(event_set_, values));
   }

   /// Number of events in this event set.
   size_t size() {
      return event_names_.size();
   }

   ///
   /// Reads a comma-separated list of event names from the specified
   /// environment variable and adds the events to this PAPIEventSet.
   /// If the architecture does not support as many counters as there are
   /// events, add any event names that couldn't be added to ignored_events.
   ///
   void add_from_environment(const std::string& env_var,
                             const char *default_events = NULL);

   /// Get the list of ignored events.
   /// Used when this is constructed with from_environment.
   const std::vector<std::string>& ignored_events() const {
      return ignored_events_;
   }

   const std::vector<std::string>& event_names() const {
      return event_names_;
   }

   /// Values of the events counted by this event set.
   /// Valid after a call to stop().
   std::vector<long long> values;

private:
   /// Convert PAPI event name to code.
   static int name_to_code(const std::string& event_name);

   /// PAPI event set handle
   int event_set_;

   /// Names of events added to the event set
   std::vector<std::string> event_names_;

   /// Names of events ignored b/c we couldn't fit them in the
   /// event set on this architecture.
   std::vector<std::string> ignored_events_;
};



#endif // PAPI_UTILS_H
