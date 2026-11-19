#ifndef GATEAU_CANDIDATES_H
#define GATEAU_CANDIDATES_H

#include <stddef.h>

struct gateau_candidate {
  const char *trigger;
  const char *input;
  const char *expected;
};

extern const struct gateau_candidate gateau_candidates[];
extern const size_t gateau_candidate_count;

#endif
