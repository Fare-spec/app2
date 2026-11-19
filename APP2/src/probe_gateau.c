#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gateau/probe.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s phrase [page-chain-limit]\n", argv[0]);
    return 2;
  }
  size_t limit = argc >= 3 ? (size_t)strtoul(argv[2], NULL, 10) : 100;
  int result = gateau_probe_once(argv[1], limit);
  const char *description = result == GATEAU_PROBE_REJECTED ? "rejected" :
                             result == GATEAU_PROBE_ACCEPTED ? "accepted" :
                             result == GATEAU_PROBE_NO_WARNING ? "no warning" :
                                                                  "error";
  printf("%s\n", description);
  return result;
}
