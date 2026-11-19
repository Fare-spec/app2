#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "appolab.h"
#include "client.h"
#include "index.h"

#define RESPONSE_CAPACITY (MAXREP + 1)
#define DEFAULT_ATTEMPT_LIMIT 10000
#define MAX_ATTEMPT_LIMIT 10000
#define MAX_CHAIN_STEPS 100
#define TOKEN_CAPACITY 64

static bool parse_attempt_limit(const char *text, size_t *limit) {
  char *end = NULL;
  unsigned long value = strtoul(text, &end, 10);
  if (end == text || *end != '\0' || value == 0 || value > MAX_ATTEMPT_LIMIT) {
    return false;
  }
  *limit = (size_t)value;
  return true;
}

static bool send_and_index(FILE *index_file, size_t *step, const char *input,
                           char *response, char answer[TOKEN_CAPACITY],
                           bool *has_answer) {
  if (!appolab_send_command(input, response)) {
    return false;
  }
  *has_answer = gateau_extract_number(response, answer, TOKEN_CAPACITY);
  gateau_index_write(index_file, (*step)++, input, *has_answer ? answer : NULL,
                     response);
  return true;
}

static bool cake_was_stepped_on(const char *response) {
  return strstr(response, "stepped on it") != NULL ||
         strstr(response, "stepped-on cake") != NULL;
}

static bool cake_was_found_safely(const char *response) {
  return strstr(response, "You found the cake!") != NULL &&
         !cake_was_stepped_on(response);
}

static int run_retry(const char *output_path, size_t attempt_limit) {
  char login[512];
  if (!appolab_read_login(login, sizeof login)) {
    return EXIT_FAILURE;
  }

  FILE *index_file = fopen(output_path, "w");
  if (index_file == NULL) {
    perror(output_path);
    return EXIT_FAILURE;
  }
  fputs("step\tinput\tanswer\tserver_reply\n", index_file);

  char response[RESPONSE_CAPACITY];
  char answer[TOKEN_CAPACITY];
  bool has_answer;
  size_t step = 0;
  int result = EXIT_FAILURE;

  show_messages(false);
  if (!connexion(NULL)) {
    fclose(index_file);
    return EXIT_FAILURE;
  }
  if (!send_and_index(index_file, &step, login, response, answer,
                      &has_answer) ||
      strstr(response, "Bienvenue") == NULL) {
    fprintf(stderr, "AppoLab login failed or connection was lost\n");
    goto finished;
  }
  if (!send_and_index(index_file, &step, "load leGateau", response, answer,
                      &has_answer) ||
      !send_and_index(index_file, &step, "depart", response, answer,
                      &has_answer) ||
      !has_answer) {
    fprintf(stderr, "Could not start leGateau or find its first number\n");
    goto finished;
  }

  if (strlen(answer) != GATEAU_CODE_WIDTH) {
    fprintf(stderr, "The first retry code is not five digits long\n");
    goto finished;
  }
  char first[GATEAU_CODE_WIDTH + 1];
  memcpy(first, answer, sizeof first);
  char current[TOKEN_CAPACITY];
  snprintf(current, sizeof current, "%s", first);
  bool first_already_sent = false;

  for (size_t attempt = 1; attempt <= attempt_limit; attempt++) {
    char seen[MAX_CHAIN_STEPS][TOKEN_CAPACITY];
    size_t seen_count = 0;
    if (first_already_sent) {
      snprintf(seen[seen_count++], TOKEN_CAPACITY, "%s", first);
      first_already_sent = false;
    }

    for (size_t chain_step = 0; chain_step < MAX_CHAIN_STEPS; chain_step++) {
      bool repeated = false;
      for (size_t i = 0; i < seen_count; i++) {
        if (strcmp(seen[i], current) == 0) {
          repeated = true;
          break;
        }
      }
      if (repeated) {
        fprintf(stderr, "Repeated code %s before the cake warning\n", current);
        goto finished;
      }
      snprintf(seen[seen_count++], TOKEN_CAPACITY, "%s", current);

      if (!send_and_index(index_file, &step, current, response, answer,
                          &has_answer)) {
        goto finished;
      }
      if (cake_was_found_safely(response)) {
        result = EXIT_SUCCESS;
        goto finished;
      }

      if (gateau_reply_needs_manual_input(response)) {
        if (!has_answer) {
          fprintf(stderr, "Careful reply did not contain a page number\n");
          goto finished;
        }

        char cake_page[TOKEN_CAPACITY];
        snprintf(cake_page, sizeof cake_page, "%s", answer);
        printf("Attempt %zu/%zu: following page %s despite the warning\n",
               attempt, attempt_limit, cake_page);
        if (!send_and_index(index_file, &step, cake_page, response, answer,
                            &has_answer)) {
          goto finished;
        }
        if (cake_was_found_safely(response)) {
          result = EXIT_SUCCESS;
          goto finished;
        }
        if (!cake_was_stepped_on(response)) {
          fprintf(stderr,
                  "The warned page did not produce the expected retry\n");
          goto finished;
        }

        if (has_answer) {
          snprintf(current, sizeof current, "%s", answer);
          continue;
        }
        if (strstr(response, "beginning") == NULL) {
          fprintf(stderr,
                  "Stepped-on reply has no continuation or restart hint\n");
          goto finished;
        }
        if (!send_and_index(index_file, &step, first, response, answer,
                            &has_answer) ||
            !has_answer) {
          fprintf(stderr, "Retry from the first number did not continue\n");
          goto finished;
        }
        printf("Restarted from %s; next number is %s\n", first, answer);
        snprintf(current, sizeof current, "%s", answer);
        if (attempt == attempt_limit) {
          printf("Reached the attempt limit (%zu)\n", attempt_limit);
          goto finished;
        }
        first_already_sent = true;
        break;
      }

      if (!has_answer) {
        fprintf(stderr, "No next number in the server reply\n");
        goto finished;
      }
      memcpy(current, answer, sizeof current);
    }
  }

finished:
  deconnexion();
  fclose(index_file);
  printf("Index file saved to %s\n", output_path);
  return result;
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 4 || strcmp(argv[1], "--run") != 0) {
    fprintf(stderr,
            "Usage: %s --run [output.tsv] [attempt-limit]\n"
            "Default attempt limit: %d; maximum: %d; one request per second\n",
            argv[0], DEFAULT_ATTEMPT_LIMIT, MAX_ATTEMPT_LIMIT);
    return EXIT_FAILURE;
  }

  const char *output_path = argc >= 3 ? argv[2] : "leGateau-retries.tsv";
  size_t attempt_limit = DEFAULT_ATTEMPT_LIMIT;
  if (argc == 4 && !parse_attempt_limit(argv[3], &attempt_limit)) {
    fprintf(stderr, "Attempt limit must be between 1 and %d\n",
            MAX_ATTEMPT_LIMIT);
    return EXIT_FAILURE;
  }
  return run_retry(output_path, attempt_limit);
}
