#include <stdio.h>
#include <string.h>

#include "gateau/candidates.h"
#include "gateau/replay.h"

int main(int argc, char **argv) {
  const char *log_path = argc >= 2 ? argv[1] : "leGateau-flow.tsv";
  printf("candidate\ttrigger\tinput\tresult\tstep\n");
  for (size_t i = 0; i < gateau_candidate_count; i++) {
    char response[512];
    size_t step = 0;
    bool passed = gateau_replay_candidate(
        log_path, gateau_candidates[i].trigger, gateau_candidates[i].input,
        gateau_candidates[i].expected, response, sizeof response, &step);
    printf("%zu\t%s\t%s\t%s\t%zu\n", i + 1, gateau_candidates[i].trigger,
           gateau_candidates[i].input, passed ? "valid" : "unverified",
           step);
  }
  return 0;
}
