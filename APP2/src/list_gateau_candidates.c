#include <stdio.h>

#include "gateau/candidates.h"

int main(void) {
  for (size_t i = 0; i < gateau_candidate_count; i++) {
    printf("%zu\t%s\t%s\n", i + 1, gateau_candidates[i].trigger,
           gateau_candidates[i].input);
  }
  return 0;
}
