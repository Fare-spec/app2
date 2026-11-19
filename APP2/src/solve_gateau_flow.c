#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "appolab.h"
#include "client.h"
#include "gateau/flow.h"

#define RESPONSE_CAPACITY (MAXREP + 1)
#define KEY_CAPACITY 1024
#define ATTEMPT_LIMIT 100000

static bool send_and_log(FILE *log_file, size_t step, const char *input,
                         char *response) {
  if (!appolab_send_command(input, response)) {
    return false;
  }
  fprintf(log_file, "%zu\t%s\t", step, input);
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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s output.tsv\n", argv[0]);
    return 1;
  }

  char login[512];
  if (!appolab_read_login(login, sizeof login)) {
    return 1;
  }
  FILE *log_file = fopen(argv[1], "w");
  if (log_file == NULL) {
    perror(argv[1]);
    return 1;
  }
  fputs("step\tinput\tserver_reply\n", log_file);

  char response[RESPONSE_CAPACITY];
  char key[KEY_CAPACITY];
  char first_key[KEY_CAPACITY];
  size_t step = 0;

  show_messages(false);
  if (!connexion(NULL)) {
    fclose(log_file);
    return 1;
  }

  bool success = false;
  if (send_and_log(log_file, step++, login, response) &&
      send_and_log(log_file, step++, "load leGateau", response) &&
      send_and_log(log_file, step++, "start", response) &&
      gateau_flow_extract_key(response, first_key, sizeof first_key)) {
    success = true;
    size_t attempt = 0;
    while (attempt < ATTEMPT_LIMIT) {
      attempt++;
      if (!send_and_log(log_file, step++, first_key, response)) {
        success = false;
        goto finished;
      }
      if (strstr(response, "Finished in") == response) {
        success = false;
        goto finished;
      }

      for (size_t chain_step = 0; chain_step < ATTEMPT_LIMIT; chain_step++) {
        if (!gateau_flow_extract_key(response, key, sizeof key)) {
          fprintf(stderr, "No recognized page key at step %zu\n", step);
          success = false;
          goto finished;
        }
        if (!send_and_log(log_file, step++, key, response)) {
          success = false;
          goto finished;
        }
        if (strstr(response, "You found the cake!") != NULL &&
            strstr(response, "stepped on it") == NULL) {
          success = true;
          goto finished;
        }
        if (strstr(response, "stepped on it") != NULL) {
          if (!send_and_log(log_file, step++, first_key, response) ||
              !gateau_flow_extract_key(response, key, sizeof key)) {
            success = false;
            goto finished;
          }
          snprintf(first_key, sizeof first_key, "%s", key);
        }
        if (strstr(response, "Finished in") == response) {
          if (strstr(response, "GlaDOS") != NULL) {
            success = true;
            goto finished;
          }
          if (!send_and_log(log_file, step++, "GlaDOS", response)) {
            success = false;
            goto finished;
          }
          if (strstr(response, "Finished in") != NULL) {
            success = true;
            goto finished;
          }
        }
      }
    }
  }

finished:
  deconnexion();
  fclose(log_file);
  printf("Flow log saved to %s\n", argv[1]);
  return success ? 0 : 1;
}
