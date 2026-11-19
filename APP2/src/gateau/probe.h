#ifndef GATEAU_PROBE_H
#define GATEAU_PROBE_H

#include <stddef.h>

enum gateau_probe_result {
  GATEAU_PROBE_REJECTED,
  GATEAU_PROBE_ACCEPTED,
  GATEAU_PROBE_NO_WARNING,
  GATEAU_PROBE_ERROR
};

int gateau_probe_once(const char *phrase, size_t page_chain_limit);

#endif
