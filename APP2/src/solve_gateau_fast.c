#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "appolab.h"
#include "client.h"
#include "gateau/flow.h"

#define RESPONSE_CAPACITY (MAXREP + 1)
#define KEY_CAPACITY 1024
#define STEP_LIMIT 11000

static double now(void) {
  struct timespec value;
  clock_gettime(CLOCK_MONOTONIC, &value);
  return value.tv_sec + value.tv_nsec / 1000000000.0;
}

static size_t exchanges;
static double network_seconds;

static bool exchange(const char *input, char *response) {
  double begin = now();
  int result = envoyer_recevoir((char *)input, response);
  network_seconds += now() - begin;
  exchanges++;
  return result >= 0;
}

int main(void) {
  char login[512];
  if (!appolab_read_login(login, sizeof login) || !gateau_flow_prepare()) {
    return 1;
  }

  char response[RESPONSE_CAPACITY] = "";
  char key[KEY_CAPACITY];
  char first_key[KEY_CAPACITY];
  bool success = false;
  bool restarted = false;
  double started = 0;
  const double checkpoints[] = {6.0, 7.04, 8.34, 9.0};
  size_t checkpoint = 0;

  show_messages(false);
  if (!connexion(NULL)) {
    return 1;
  }
  if (!exchange(login, response) || strstr(response, "Bienvenue") == NULL ||
      !exchange("load leGateau", response) ||
      strstr(response, "Will you find the cake?") == NULL) {
    fprintf(stderr, "Could not log in or load leGateau\n");
    goto finished;
  }

  exchanges = 0;
  network_seconds = 0;
  started = now();
  if (!exchange("start", response) ||
      !gateau_flow_extract_key(response, first_key, sizeof first_key)) {
    goto finished;
  }

  for (size_t step = 0; step < STEP_LIMIT; step++) {
    double elapsed = now() - started;
    if (checkpoint < sizeof checkpoints / sizeof checkpoints[0] &&
        elapsed >= checkpoints[checkpoint]) {
      fprintf(stderr, "Checkpoint %.2f s: actual %.6f s, %zu exchanges; %s\n",
              checkpoints[checkpoint++], elapsed, exchanges, response);
    }
    if (exchanges % 1000 == 0) {
      fprintf(stderr, "Progress: %zu exchanges in %.3f s (%.3f ms/exchange)\n",
              exchanges, elapsed, 1000 * network_seconds / exchanges);
    }

    if (strncmp(response, "Finished in", 11) == 0) {
      printf("Cake response: %s\n", response);
      if (!exchange("GlaDOS", response)) {
        fprintf(stderr, "Could not send the final GlaDOS command\n");
        goto finished;
      }
      printf("Final GlaDOS response: %s\n", response);
      success = true;
      goto finished;
    }

    if (strstr(response, "stepped on it") != NULL) {
      if (restarted) {
        fprintf(stderr, "Unexpected second restart\n");
        goto finished;
      }
      restarted = true;
      snprintf(key, sizeof key, "%s", first_key);
    } else if (!gateau_flow_extract_key(response, key, sizeof key)) {
      fprintf(stderr, "Unrecognized reply at exchange %zu: %s\n",
              exchanges, response);
      goto finished;
    }
    if (!exchange(key, response)) {
      goto finished;
    }
  }
  fprintf(stderr, "Step limit reached without getting the cake\n");

finished:
  if (started != 0) {
    double elapsed = now() - started;
    printf("%s; local elapsed %.6f s, %zu exchanges\n"
           "Time inside envoyer_recevoir: %.6f s (mean %.3f ms); "
           "other client time: %.6f s\n",
           success ? "Completed" : "Failed", elapsed, exchanges,
           network_seconds, exchanges ? network_seconds * 1000 / exchanges : 0,
           elapsed - network_seconds);
  }
  deconnexion();
  return success ? 0 : 1;
}
