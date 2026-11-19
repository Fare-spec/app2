#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "appolab.h"
#include "client.h"
#include "index.h"

#define RESPONSE_CAPACITY (MAXREP + 1)
#define DEFAULT_STEP_LIMIT 10000
#define MANUAL_TRY_LIMIT 10

static bool parse_step_limit(const char *text, size_t *limit) {
  char *end = NULL;
  errno = 0;
  unsigned long value = strtoul(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0' || value == 0 ||
      value > GATEAU_CODE_COUNT) {
    return false;
  }
  *limit = (size_t)value;
  return true;
}

static bool manual_input_loop(FILE *index_file, size_t first_step,
                              char *response) {
  char input[MAXREP + 2];
  size_t step = first_step;
  size_t attempts = 0;

  while (attempts < MANUAL_TRY_LIMIT) {
    printf("Next server input (%zu/%d; Ctrl-D to stop, empty line is sent): ",
           attempts + 1,
           MANUAL_TRY_LIMIT);
    fflush(stdout);
    if (fgets(input, sizeof input, stdin) == NULL) {
      return true;
    }

    size_t length = strcspn(input, "\r\n");
    bool complete_line = input[length] != '\0' || feof(stdin);
    input[length] = '\0';
    if (!complete_line) {
      int ch;
      while ((ch = getchar()) != '\n' && ch != EOF) {
      }
      fprintf(stderr, "Input is too long; enter at most %d characters\n", MAXREP);
      continue;
    }
    if (!appolab_send_command(input, response)) {
      fprintf(stderr, "Connection lost after manual input\n");
      return false;
    }

    char answer[GATEAU_CODE_WIDTH + 1];
    bool has_answer = gateau_extract_code(response, answer);
    gateau_index_write(index_file, step++, input,
                       has_answer ? answer : NULL, response);
    attempts++;
  }
  printf("Manual input limit reached (%d submissions).\n", MANUAL_TRY_LIMIT);
  return true;
}

static int run_indexer(const char *output_path, size_t step_limit) {
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
  fflush(index_file);

  char response[RESPONSE_CAPACITY];
  bool ok = false;
  show_messages(false);
  if (!connexion(NULL)) {
    fclose(index_file);
    return EXIT_FAILURE;
  }

  if (!appolab_send_command(login, response) ||
      strstr(response, "Bienvenue") == NULL) {
    fprintf(stderr, "AppoLab login failed or connection was lost\n");
    goto finished;
  }
  if (!appolab_send_command("load leGateau", response)) {
    fprintf(stderr, "Connection lost while loading leGateau\n");
    goto finished;
  }
  gateau_index_write(index_file, 0, "load leGateau", NULL, response);
  if (!appolab_send_command("depart", response)) {
    fprintf(stderr, "Connection lost while starting leGateau\n");
    goto finished;
  }

  char current[GATEAU_CODE_WIDTH + 1];
  if (!gateau_extract_code(response, current)) {
    fprintf(stderr, "No five-digit number in the depart reply\n");
    goto finished;
  }
  gateau_index_write(index_file, 1, "depart", current, response);

  bool seen[GATEAU_CODE_COUNT] = {false};
  for (size_t submitted = 1; submitted <= step_limit; submitted++) {
    unsigned int current_index = gateau_code_index(current);
    if (seen[current_index]) {
      printf("Cycle detected at %s\n", current);
      ok = true;
      break;
    }
    seen[current_index] = true;

    if (!appolab_send_command(current, response)) {
      fprintf(stderr, "Connection lost after submitting %s\n", current);
      goto finished;
    }

    char next[GATEAU_CODE_WIDTH + 1];
    bool careful_reply = gateau_reply_needs_manual_input(response);
    bool has_next = gateau_extract_code(response, next);
    gateau_index_write(index_file, submitted + 1, current,
                       has_next ? next : NULL, response);
    printf("%zu: %s -> %s\n", submitted, current,
           has_next ? next : "(no next number)");
    if (careful_reply) {
      printf("Careful reply after input %s. Manual input mode; no automatic command will be sent.\n",
             current);
      ok = manual_input_loop(index_file, submitted + 2, response);
      break;
    }
    if (!has_next) {
      ok = true;
      break;
    }
    memcpy(current, next, sizeof current);
    if (submitted == step_limit) {
      fprintf(stderr, "Step limit reached; increase it to continue\n");
    }
  }

finished:
  deconnexion();
  fclose(index_file);
  printf("Index file saved to %s\n", output_path);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 4 || strcmp(argv[1], "--run") != 0) {
    fprintf(stderr,
            "Usage: %s --run [output.tsv] [step-limit]\n"
            "Default output: leGateau-index.tsv; default step limit: %d\n",
            argv[0], DEFAULT_STEP_LIMIT);
    return EXIT_FAILURE;
  }

  const char *output_path = argc >= 3 ? argv[2] : "leGateau-index.tsv";
  size_t step_limit = DEFAULT_STEP_LIMIT;
  if (argc == 4 && !parse_step_limit(argv[3], &step_limit)) {
    fprintf(stderr, "Step limit must be between 1 and %d\n",
            GATEAU_CODE_COUNT);
    return EXIT_FAILURE;
  }
  return run_indexer(output_path, step_limit);
}
