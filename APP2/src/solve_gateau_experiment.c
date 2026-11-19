#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "appolab.h"
#include "client.h"
#include "gateau/flow.h"

#define RESPONSE_CAPACITY (MAXREP + 1)
#define KEY_CAPACITY 1024
static bool exchange(const char *input, char *response) {
  return envoyer_recevoir((char *)input, response) >= 0;
}

#define CHAIN_LIMIT 10000

static bool log_exchange(FILE *log_file, size_t *step, const char *input,
                         char *response) {
  if (!exchange(input, response)) {
    return false;
  }
  fprintf(log_file, "%zu\t%s\t", ++*step, input);
  for (const unsigned char *cursor = (const unsigned char *)response;
       *cursor != '\0'; cursor++) {
    fputc(*cursor == '\n' || *cursor == '\r' || *cursor == '\t' ? ' '
                                                                : *cursor,
          log_file);
  }
  fputc('\n', log_file);
  fflush(log_file);
  return true;
}

static bool starts_with(const char *response, const char *prefix) {
  return strncmp(response, prefix, strlen(prefix)) == 0;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s experiment-input\n", argv[0]);
    return 2;
  }

  char login[512];
  if (!appolab_read_login(login, sizeof login)) {
    return 2;
  }

  char response[RESPONSE_CAPACITY];
  char key[KEY_CAPACITY];
  char first_key[KEY_CAPACITY];
  char previous_key[KEY_CAPACITY] = "";
  char current_key[KEY_CAPACITY] = "";
  bool first_restart_done = false;
  size_t step = 0;
  FILE *log_file = fopen("leGateau-experiment.tsv", "w");
  if (log_file == NULL) {
    return 2;
  }

  show_messages(false);
  if (!connexion(NULL)) {
    return 2;
  }

  struct timespec start_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  bool success = false;
  if (log_exchange(log_file, &step, login, response) &&
      log_exchange(log_file, &step, "load leGateau", response) &&
      log_exchange(log_file, &step, "start", response) &&
      gateau_flow_extract_key(response, first_key, sizeof first_key)) {
    if (!log_exchange(log_file, &step, first_key, response)) {
      goto finished;
    }

    for (size_t chain_step = 0; chain_step < CHAIN_LIMIT; chain_step++) {
      if (strstr(response, "stepped on it") != NULL) {
        if (first_restart_done ||
            !log_exchange(log_file, &step, first_key, response)) {
          goto finished;
        }
        first_restart_done = true;
      }

      if (strstr(response, "Let us spice things up") != NULL) {
        const char *choice = NULL;
        if (strcmp(argv[1], "previous") == 0) {
          choice = previous_key;
        } else if (strcmp(argv[1], "current") == 0) {
          choice = current_key;
        } else if (strcmp(argv[1], "first") == 0) {
          choice = first_key;
        } else {
          choice = argv[1];
        }
        if (!log_exchange(log_file, &step, choice, response)) {
          goto finished;
        }
        if (strcmp(argv[1], "first-twice") == 0 &&
            !log_exchange(log_file, &step, choice, response)) {
          goto finished;
        }
        success = starts_with(response, "Finished in");
        goto finished;
      }

      if (!gateau_flow_extract_key(response, key, sizeof key) ||
          !log_exchange(log_file, &step, key, response)) {
        goto finished;
      }
      snprintf(previous_key, sizeof previous_key, "%s", current_key);
      snprintf(current_key, sizeof current_key, "%s", key);
    }
  }

finished:;
  struct timespec finish_time;
  clock_gettime(CLOCK_MONOTONIC, &finish_time);
  double elapsed = (finish_time.tv_sec - start_time.tv_sec) +
                   (finish_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
  printf("Experiment: %s\nResult: %s\nElapsed: %.9f seconds\nResponse: %s\n",
         argv[1], success ? "accepted" : "rejected", elapsed, response);
  deconnexion();
  fclose(log_file);
  return success ? 0 : 1;
}
